#include "requestData.h"
#include <iostream>
#include <unordered_map>
#include "epoll.h"
#include "util.h"
#include <unistd.h>
#include <sys/time.h>
#include <queue>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <string.h>
#include <queue>

using namespace std;

pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MimeType::lock = PTHREAD_MUTEX_INITIALIZER;
unordered_map<string, string>MimeType::mime;

string MimeType::getMime(const std::string &suffix){
	if(mime.size() == 0){
		pthread_mutex_lock(&lock);
		if(mime.size() == 0){
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
		pthread_mutex_unlock(&lock);
	}
	if(mime.find(suffix) == mime.end()){
		return mime["default"];
	}
	else
		return mime[suffix];
}

priority_queue<shared_ptr<mytimer>, deque<shared_ptr<mytimer>>, timerCmp> myTimerQueue;

requestData::requestData():againTimes(0),now_read_pos(0),state(STATE_PARSE_URI),h_state(h_start),keep_alive(false){
	cout<<"requestData constructed!";
}

requestData::requestData(int _epollfd, int _fd, std::string _path):againTimes(0),now_read_pos(0),state(STATE_PARSE_URI),h_state(h_start),keep_alive(false),path(_path), fd(_fd), epollfd(_epollfd){
	cout<<"requestData()"<<endl;
}

requestData::~requestData(){
	cout<<"~requestData()"<<endl;
}

void requestData::addTimer(shared_ptr<mytimer> mtimer){
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
	content.clear();
	file_name.clear();
	now_read_pos = 0;
	state = STATE_PARSE_URI;
	h_state = h_start;
	keep_alive = false;
}

void requestData::seperateTime(){
	if(timer.lock()){
		shared_ptr<mytimer> my_timer(timer.lock());
		my_timer->clearReq();
		timer.reset();
	}
}

void requestData::handleRequest(){
	char buff[MAX_BUFF];
	bool isError = false;
	while(true){
		int read_num = readn(fd, buff, MAX_BUFF);
		if(read_num < 0){
			perror("1");
			isError = true;
			break;
		}
		else if(read_num == 0){
			perror("read_num == 0");
			if(errno == EAGAIN){
				if(againTimes > AGAIN_MAX_TIMES)
					isError = true;
				else
					++againTimes;
			}
			else if(errno != 0)
				isError = true;
			break;
		}
		string now_read(buff, buff + read_num);
		content += now_read;

		if(state == STATE_PARSE_URI){
			int flag = this->parse_URI();
			if(flag == PARSE_URI_AGAIN)
				break;
			else if(flag == PARSE_URI_ERROR){
				perror("2");
				isError = true;
				break;
			}
		}
		if(state == STATE_PARSE_HEADERS){
			int flag = this->parse_Headers();	
			if(flag == PARSE_HEADER_AGAIN){
				break;
			}
			else if(flag == PARSE_HEADER_ERROR){
				perror("3");
				isError = true;
				break;
			}
			if(method == METHOD_POST){
				state = STATE_RECV_BODY;
			}
			else{
				state = STATE_ANALYSIS;
			}
		}
		if(state == STATE_RECV_BODY){
			int content_length = -1;
			if(headers.find("Content-length") != headers.end()){
				content_length = stoi(headers["Content-length"]);
			}
			else{
				isError = true;
				break;
			}
			if(content.size() < content_length)
				continue;
			state = STATE_ANALYSIS;
		}
		if(state == STATE_ANALYSIS){
			int flag = this->analysisRequest();
			if(flag < 0){
				isError = true;
				break;
			}	
			else if(flag == ANALYSIS_SUCCESS){
				state = STATE_FINISH;		
				break;
			}
			else{
				isError = true;
				break;
			}
		}
	}
	if(isError == true){
		delete this;
		return;
	}

	if(state == STATE_FINISH){
		if(keep_alive){
			printf("ok\n");
			this->reset();
		}
		else{
			delete this;
			return;
		}
	}
	pthread_mutex_lock(&qlock);
	shared_ptr<mytimer> mtimer(new mytimer(shared_from_this(), 500));
	this->addTimer(mtimer);
	{
		MutexLockGuard lock;		
		myTimerQueue.push(mtimer);
	}
	__uint32_t _epo_event = EPOLLIN | EPOLLET | EPOLLONESHOT;
	int ret = Epoll::epoll_mod(fd, shared_from_this(), _epo_event);
	if(ret < 0){
		delete this;
		return;
	}
}

int requestData::parse_URI(){
	string &str = content;
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
	string &str = content;
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
		char header[MAX_BUFF];
		sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
		if(headers.find("Connection") != headers.end()){
			keep_alive = true;
			sprintf(header, "%sConnection: keep_alive\r\n", header);
			sprintf(header, "%skeep_Alive: timeout=%d\r\n", header);
		}
		char *send_content = "I have received this.";
		
		sprintf(header, "%sContent-length: %zu\r\n", header, strlen(send_content));
		sprintf(header, "%s\r\n", header);
		size_t send_len = (size_t)writen(fd, header, strlen(header));
		if(send_len != strlen(header)){
			perror("Send header failed.\n");
			return ANALYSIS_ERROR;
		}
		
		send_len = (size_t)writen(fd, send_content, strlen(send_content));
		if(send_len != strlen(send_content)){
			perror("Send content failed.\n");
			return ANALYSIS_ERROR;
		}
		cout<<"content size == " << content.size() <<endl;
		return ANALYSIS_SUCCESS;
	}
	else if(method == METHOD_GET){
		char header[MAX_BUFF];
		sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
		if(headers.find("Connection") != headers.end() && headers["Connection"] == "keep-alive"){
			keep_alive = true;
			sprintf(header, "%sConnection: keep-alive\r\n", header);
			sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, EPOLL_WAIT_TIME);
		}
		int dot_pos = file_name.find('.');
		const char* filetype;
		if(dot_pos < 0)
			filetype = MimeType::getMime("default").c_str();
		else
			filetype = MimeType::getMime(file_name.substr(dot_pos)).c_str();
		struct stat sbuf;
		if(stat(file_name.c_str(), &sbuf) < 0){
			handleError(fd, 404, "Not Found!");
			return ANALYSIS_ERROR;
		}
		sprintf(header, "%sContent-type: %s\r\n", header, filetype);
		
		sprintf(header, "%s\r\n", header);
		size_t send_len = (size_t)writen(fd, header, strlen(header));
		if(send_len != strlen(header)){
			perror("Send header failed.\n");
			return ANALYSIS_ERROR;
		}
		int src_fd = open(file_name.c_str(), O_RDONLY, 0);
		char *src_addr = static_cast<char*>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
		close(src_fd);

		send_len = writen(fd, src_addr, sbuf.st_size);
		if(send_len != sbuf.st_size){
			perror("Send file failed.\n");
			return ANALYSIS_ERROR;
		}
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

mytimer::mytimer(std::shared_ptr<requestData> _request_data, int timeout): deleted(false), request_data(_request_data){
	struct timeval now;
	gettimeofday(&now, NULL);
	expired_time = ((now.tv_sec * 1000) + (now.tv_usec / 1000))+timeout;
}

mytimer::~mytimer(){
	cout<<"~mytimer()\n";
	
}

bool mytimer::isvalid(){
	struct timeval now;
	gettimeofday(&now, NULL);
	size_t temp = ((now.tv_sec * 1000) + (now.tv_usec / 1000));
	if(temp < expired_time)
		return true;
	else{
		this->setDeleted();
		return false;
	}
}

void mytimer::clearReq(){
	request_data = NULL;
	this->setDeleted();
}

void mytimer::setDeleted(){
	deleted = true;
}

bool mytimer::isDeleted()const{
	return deleted;
}

size_t mytimer::getExpTime()const{
	return expired_time;
}

bool timerCmp::operator()(shared_ptr<mytimer>a, shared_ptr<mytimer>b) const{
	return a->getExpTime() > b->getExpTime();
}

MutexLockGuard::MutexLockGuard(){
	pthread_mutex_lock(&lock);
}

MutexLockGuard::~MutexLockGuard(){
	pthread_mutex_unlock(&lock);
}

