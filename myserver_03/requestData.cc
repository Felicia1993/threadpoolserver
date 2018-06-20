#include "requestData.h"
#include "epoll.h"
#include "util.h"
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <sys/time.h>
#include <queue>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <cstdlib>
#include <queue>
#include <string.h>
using namespace std;

pthread_once_t MimeType::once_control = PTHREAD_ONCE_INIT;
unordered_map<string, string>MimeType::mime;

void MimeType::init(){
	mime[".html"] = "text/html";
	mime[".avi"] = "video/x-msvideo";
	mime[".doc"] = "application/msword";
	mime[".gz"] = "application/x-gzip";
	mime[".png"] = "image/png";
	mime[".jpg"] = "image/jpg";
	mime[".ico"] = "application/x-ico";
	mime[".gif"] = "image/gif";
	mime["htm"] = "text/html";
	mime[".avi"] = "video/x-msvideo";
	mime[".bmp"] = "image/bmp";
	mime[".c"] = "text/plain";
	mime[".mp3"] = "audio/mp3";
	mime[".txt"] = "text/plain";
	mime["default"] = "text/html";
 	
}

std::string MimeType::getMime(const std::string &suffix){
	pthread_once(&once_control, MimeType::init);
	if(mime.find(suffix) == mime.end())
		return mime["default"];
	else
		return mime[suffix];
}


requestData::requestData():againTimes(0),now_read_pos(0),state(STATE_PARSE_URI),h_state(h_start),keep_alive(false){
	cout<<"requestData constructed!";
}

requestData::requestData(int _epollfd, int _fd, std::string _path):againTimes(0),now_read_pos(0),state(STATE_PARSE_URI),h_state(h_start),keep_alive(false),path(_path), fd(_fd), epollfd(_epollfd){
	cout<<"requestData()"<<endl;
}

requestData::~requestData(){
	cout<<"~requestData()"<<endl;
}

void requestData::LinkTimer(std::shared_ptr<TimeNode>mtimer){
	timer = mtimer;
}

int requestData::getFd(){
	return fd;
}

void requestData::setFd(int _Fd){
	fd = _Fd;
}

void requestData::reset(){
	againTimes = 0;
	path.clear();
	inBuffer.clear();
	file_name.clear();
	now_read_pos = 0;
	state = STATE_PARSE_URI;
	h_state = h_start;
	headers.clear();
	keep_alive = false;
	if(timer.lock()){
		shared_ptr<TimeNode> my_timer(timer.lock());
		my_timer->clearReq();
		timer.reset();
	}
}

void requestData::seperateTime(){
	if(timer.lock()){
		shared_ptr<TimeNode> my_timer(timer.lock());
		my_timer->clearReq();
		timer.reset();
	}
}

void requestData::handleRead(){
	do{
		int read_num = readn(fd, inBuffer);
		printf("read_num = %d\n", read_num);
		if(read_num < 0){
			perror("1");
			error = true;
			handleError(fd, 400, "Bad Request");
			break;
		}
		else if(read_num == 0){
			error = true;
			break;
		}
		if(state == STATE_PARSE_URI){
			int flag = this->parse_URI();
			if(flag == PARSE_URI_AGAIN)
				break;
			else if(flag == PARSE_URI_ERROR){
				perror("2");
				error = true;
				handleError(fd, 400, "Bad Request");
				break;
			}
			else
				state = STATE_PARSE_HEADERS;
		}
		if(state == STATE_PARSE_HEADERS){
			int flag = this->parse_Headers();
			if(flag == PARSE_HEADER_AGAIN)
				break;
			else if(flag == PARSE_HEADER_ERROR){
				perror("3");
				error = true;
				handleError(fd, 400, "Bad Request");
				break;
			}
			if(method == METHOD_POST){
				state = STATE_RECV_BODY;
			}
			if(method == METHOD_GET){
				state = STATE_ANALYSIS;
			}
		}
		if(state == STATE_RECV_BODY){
			int content_length = -1;
			if(headers.find("Content-length")!=headers.end()){
				content_length = stoi(headers["Content-length"]);
			}
			else{
				error = true;
				handleError(fd, 400, "Bad Request: Lack of argument (Content-length)");
				break;
			}
			if(inBuffer.size() < content_length)
				break;
			state = STATE_ANALYSIS;
		}
		if(state == STATE_ANALYSIS){
			int flag = this->analysisRequest();
			if(flag == ANALYSIS_SUCCESS){
				state = STATE_FINISH;
				break;	
			}	
			else{
				error = true;
				break;
			}
		}
	}while(false);
	if(!error){
		if(outBuffer.size() < 0)
			events |= EPOLLOUT;
		if(state == STATE_FINISH){
			cout<<"keep-alive = "<<keep_alive<<endl;
			if(keep_alive){
				this->reset();
				events |= EPOLLIN;
			}
			else
				return;
		}
		else		
			events |= EPOLLIN;
	}
}

void requestData::handleWrite(){
	if(!error){
		if(writen(fd, outBuffer) < 0){
			perror("written");
			events = 0;
			error = true;
		}
		else if(outBuffer.size() < 0)
			events |= EPOLLOUT;
	}
}

void requestData::handleConn(){
	if(!error){
		if(events != 0){
			int timeout = 2000;
			if(keep_alive){
				timeout = 5*60*1000;				
			}
			isAbleRead = false;
			isAbleWrite = false;
			Epoll::add_timer(shared_from_this(), timeout);
			if((events & EPOLLIN) && (events & EPOLLOUT)){
				events = __uint32_t(0);
				events |= EPOLLOUT;
			}
			events |= (EPOLLET | EPOLLONESHOT);
			__uint32_t _events = events;
			events = 0;
			if(Epoll::epoll_mod(fd, shared_from_this(), _events) < 0){
				printf("Epoll::epoll_mod failed.\n");
			}
		}
		else if(keep_alive){
			events |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
			int timeout = 5 * 60 * 1000;
			isAbleRead = false;
			isAbleWrite = false;
			Epoll::add_timer(shared_from_this(), timeout);
			__uint32_t _events = events;
			events = 0;
			if(Epoll::epoll_mod(fd, shared_from_this(), _events) < 0)
				printf("Epoll::epoll_mod error\n");
		}
	}
}

int requestData::parse_URI(){
	string &str = inBuffer;
	int pos = str.find('\r',now_read_pos);
	if(pos < 0){
		return PARSE_URI_AGAIN;
	}
	
	string request_line = str.substr(0, pos);
	if(str.size() > pos + 1)
		str = str.substr(pos+1);
	else
		str.clear();
	
	pos = request_line.find("GET");	
	if(pos < 0){
		pos = request_line.find("POST");
		if(pos < 0)
			return PARSE_URI_ERROR;
		else
			method = METHOD_POST;
	}
	else
		method = METHOD_GET;
		
	pos = request_line.find("/", pos);
	if(pos < 0)
		return PARSE_URI_ERROR;
	else{
		int _pos = request_line.find(' ',pos);
		if(_pos < 0)
			return PARSE_URI_ERROR;
		else{
			if(_pos - pos > 1){
				file_name = request_line.substr(pos+1, _pos-pos-1);
				int __pos = file_name.find('?');
				if(__pos >= 0){
					file_name = file_name.substr(0, __pos);
				}
			}
			else
				file_name = "index.html";
		}
		pos = _pos;
	}
	pos = request_line.find("/", pos);
	if(pos < 0){
		return PARSE_URI_ERROR;
	}
	else{
		if(request_line.size() - pos <= 3){
			return PARSE_URI_ERROR;
		}
		else{
			string ver = request_line.substr(pos + 1, 3);
			if(ver == "1.0")
				HTTPversion = HTTP_10;
			else if(ver == "1.1")
				HTTPversion = HTTP_11;
			else
				return PARSE_URI_ERROR;
		}
	}
	state = STATE_PARSE_HEADERS;
	return PARSE_URI_SUCCESS;		
}

int requestData::parse_Headers(){
	string &str = inBuffer;
	int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
	int now_read_line_begin = 0;
	bool notFinish = true;
	for(int i = 0; i < str.size() && notFinish; ++i){
		switch(h_state){
			case h_start:
			{
				if(str[i] == '\n' || str[i] == '\r')
					break;
				h_state = h_key;
				key_start = i;
				now_read_line_begin = i;
				break;
			}		
			case h_key:
			{
				if(str[i] == ':'){
					key_end = i;
					if(key_end - key_start <= 0)
						return PARSE_HEADER_ERROR;
					h_state = h_colon;
				}
				else if(str[i] == '\n' || str[i] =='\r')
					return PARSE_HEADER_ERROR;
				break;
			}
			case h_colon:	
			{
				if(str[i] == ' ')
					h_state = h_spaces_after_colon;
				else 
					return PARSE_HEADER_ERROR;
				break;
			}
			case h_spaces_after_colon:{
				h_state = h_value;
				value_start = i;
				break;
			}
			case h_value:{
				if(str[i] == '\r'){
					h_state = h_CR;
					value_end = i;
					if(value_end - value_start <= 0)
						return PARSE_HEADER_ERROR;
				}
				else  if(i - value_start > 255)
					return PARSE_HEADER_ERROR;
				break;
			}
			case h_CR:{
				if(str[i] == '\n'){
					h_state = h_LF;
					string key(str.begin()+key_start, str.begin()+key_end);
					string value(str.begin() + value_start, str.end() + value_end);
					headers[key] = value;
					now_read_line_begin = i;
				}
				else
					return PARSE_HEADER_ERROR;
				break;
			}
			case h_LF:{
				if(str[i] == '\r')
					h_state = h_end_CR;
				else{
					key_start = i;
					h_state = h_key;
				}
				break;
			}
			case h_end_CR:{
				if(str[i] == '\n'){
					h_state = h_end_LF;
				}
				else
					return PARSE_HEADER_ERROR;
				break;
			}
			case h_end_LF:{
				notFinish = false;
				key_start = i;
				now_read_line_begin = i;
				break;
			}
		}
	}
	if(h_state == h_end_LF){
		str = str.substr(now_read_line_begin);
		return PARSE_HEADER_SUCCESS;
	}
	str = str.substr(now_read_line_begin);
	return PARSE_HEADER_AGAIN;
}

int requestData::analysisRequest(){
	if(method == METHOD_POST){
		string header;
		header += "HTTP/1.1 200 OK\r\n";
		if(headers.find("Connection") != headers.end()){
			keep_alive = true;
			header += string("Connection:keep_alive\r\n") + "keep_Alive: timeout="+to_string(5*60*1000)+"\r\n";
		}
		string send_content = "I have received this.";
		int length = stoi(headers["Content-length"]);
		vector<char> data(inBuffer.begin(), inBuffer.begin() + length);
		cout<<"data.size = "<<data.size()<<endl;
		return ANALYSIS_SUCCESS;
	}
	else if(method == METHOD_GET){
		string header;
		header= string("HTTP/1.1 200 OK ");
		if(headers.find("Connection") != headers.end() && headers["Connection"] == "keep-alive"){
			keep_alive = true;
			header += string("Connection:keep_alive\r\n") + "keep_Alive: timeout="+to_string(5*60*1000)+"\r\n";
		}
		int dot_pos = file_name.find('.');
		string filetype;
		if(dot_pos < 0)
			filetype = MimeType::getMime("default");
		else
			filetype = MimeType::getMime(file_name.substr(dot_pos));
		struct stat sbuf;
		if(stat(file_name.c_str(), &sbuf) < 0){
			handleError(fd, 404, "Not Found!");
			return ANALYSIS_ERROR;
		}
		header += "Content-type: " + filetype + "\r\n";
		header += "Content-length: " + to_string(sbuf.st_size) + "\r\n";

		header += "\r\n";
		outBuffer += header;

		int src_fd = open(file_name.c_str(), O_RDONLY, 0);
		char *src_addr = static_cast<char*>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
		close(src_fd);

		outBuffer += src_addr;
		munmap(src_addr, sbuf.st_size);
		return ANALYSIS_SUCCESS;
	}
	else
		return ANALYSIS_ERROR;
}

void requestData::handleError(int fd, int err_num, string short_msg){
	short_msg = " " + short_msg;
	char send_buff[MAX_BUFF];
	string body_buff, header_buff;
	body_buff += "<html><title>TKeed Error</title>";
	body_buff += "<body bgcolor=\"ffffff\">";
	body_buff += to_string(err_num) + short_msg;
	body_buff += "<hr><em> LinYa's Web Server</em>\n</body></html>";
	
	header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
	header_buff += "Content-type: text/html\r\n";
	header_buff += "Connection: close\r\n";
	header_buff += "Connect-length: " + to_string(body_buff.size()) + "\r\n";
	header_buff += "\r\n";
	sprintf(send_buff, "%s", header_buff.c_str());
	writen(fd, send_buff, strlen(send_buff));
	sprintf(send_buff, "%s", body_buff.c_str());
	writen(fd, send_buff, strlen(send_buff));
}

void requestData::disableReadAndWrite(){
	isAbleRead = false;
	isAbleWrite = false;
}

void requestData::enableRead(){
	isAbleRead = true;
}

void requestData::enableWrite(){
	isAbleWrite = true;
}

bool requestData::canRead(){
	return isAbleRead;
}

bool requestData::canWrite(){
	return isAbleWrite;
}


