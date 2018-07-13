#include "HttpData.h"

const __uint32_t DEFAULT_EVENT = EPOLLIN|EPOLLET|EPOLLONESHOT;
const int DEFAULT_EXPIRED_TIME = 2000;//ms
const int DEFAULT_KEEP_ALIVE_TIME = 5*60*1000;//ms

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

HttpData::HttpData(EventLoop* loop, int connfd):loop_(loop),fd_(connfd),channel_(new Channel(loop, connfd)),method_(METHOD_GET),HTTPversion_(HTTP_11),state_(STATE_PARSE_URI), h_state_(H_START),keep_alive(false),error_(false),connectionState_(H_CONNECTED),{
	channel_->setReadCallback(bind(&HttpData::handleread,this));
	channel_->setWriteCallback(bind(&HttpData::handlewrite,this));
	channel_->setConnCallback(bind(&HttpData::handleConn,this));
}

HttpData::~HttpData(){
	
}

void HttpData::reset(){
	filename_.clear();
	path_.clear();
	nowReadPos_ = 0;
	h_state_ = H_START;
	state_ = STATE_PARSE_URI;
	headers_.clear();
	
	if(timer_.lock()){
		shared_ptr<TimerNode> my_timer(timer_.lock());
		my_timer->clearReq();
		timer_.reset();
	}
}

void HttpData::seperateTimer(){
	if(timer_.lock()){
		shared_ptr<TimerNode> my_timer(timer_.lock());
		my_timer->clearReq();
		timer_.reset();
	}
}

void HttpData::handleClose(){
	connectionState_ = H_DISCONNECTED;
	shared_ptr<HttpData> guard(shared_from_this);
	loop_->removeFromPoller(channel_);
}

void HttpData::newEvent(){
	channel_->setEvents(EPOLLIN | EPOLLET);
	loop_->addToPoller(channel_, DEFAULT_EXPIRED_TIME);
}

HeaderState HttpData::parse_Headers(){
	string &str = inBuffer_;
	int key_start = -1, key_end = -1, value_start = -1, value_end = -1;
	int now_read_line_begin = 0;
	bool notFinish = true;
	for(int i = 0; i < str.size() && notFinish; ++i){
		switch(h_state){
			case H_START:
			{
				if(str[i] == '\n' || str[i] == '\r')
					break;
				H_START = H_KEY;
				key_start = i;
				now_read_line_begin = i;
				break;
			}		
			case H_KEY:
			{
				if(str[i] == ':'){
					key_end = i;
					if(key_end - key_start <= 0)
						return PARSE_HEADER_ERROR;
					H_STATE = H_COLON;
				}
				else if(str[i] == '\n' || str[i] =='\r')
					return PARSE_HEADER_ERROR;
				break;
			}
			case H_COLON:	
			{
				if(str[i] == ' ')
					H_STATE = h_SPACE_AFTER_COLON;
				else 
					return PARSE_HEADER_ERROR;
				break;
			}
			case h_SPACE_AFTER_COLON:{
				H_STATE = H_VALUE;
				value_start = i;
				break;
			}
			case H_VALUE:{
				if(str[i] == '\r'){
					H_STATE = H_CR;
					value_end = i;
					if(value_end - value_start <= 0)
						return PARSE_HEADER_ERROR;
				}
				else  if(i - value_start > 255)
					return PARSE_HEADER_ERROR;
				break;
			}
			case H_CR:{
				if(str[i] == '\n'){
					H_STATE = H_LF;
					string key(str.begin()+key_start, str.begin()+key_end);
					string value(str.begin() + value_start, str.end() + value_end);
					headers[key] = value;
					now_read_line_begin = i;
				}
				else
					return PARSE_HEADER_ERROR;
				break;
			}
			case H_LF:{
				if(str[i] == '\r')
					H_STATE = H_END_CR;
				else{
					key_start = i;
					H_STATE = H_KEY;
				}
				break;
			}
			case H_END_CR:{
				if(str[i] == '\n'){
					H_STATE = H_END_LF;
				}
				else
					return PARSE_HEADER_ERROR;
				break;
			}
			case H_END_LF:{
				notFinish = false;
				key_start = i;
				now_read_line_begin = i;
				break;
			}
		}
	}
	if(H_STATE == H_END_LF){
		str = str.substr(now_read_line_begin);
		return PARSE_HEADER_SUCCESS;
	}
	str = str.substr(now_read_line_begin);
	return PARSE_HEADER_AGAIN;
}

AnalysisState HttpData::analysisRequest(){
	if(method == METHOD_POST){
		string header;
		header += "HTTP/1.1 200 OK\r\n";
		if(headers.find("Connection") != headers.end()){
			keep_alive = true;
			header += string("Connection:keep_alive\r\n") + "keep_Alive: timeout="+to_string(5*60*1000)+"\r\n";
		}
		string send_content = "I have received this.";
		int length = stoi(headers["Content-length"]);
		vector<char> data(inBuffer_.begin(), inBuffer_.begin() + length);
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
		outBuffer_ += header;

		int src_fd = open(file_name.c_str(), O_RDONLY, 0);
		char *src_addr = static_cast<char*>(mmap(NULL, sbuf.st_size, PROT_READ, MAP_PRIVATE, src_fd, 0));
		close(src_fd);

		outBuffer_ += src_addr;
		munmap(src_addr, sbuf.st_size);
		return ANALYSIS_SUCCESS;
	}
	else
		return ANALYSIS_ERROR;
}

void HttpData::handleread(){
	__uint32_t &events_ = channel_->getEvents();
	do{
		bool zero = false;
		int read_num = read(fd, inBuffer_, zero);
		LOG<<"Request: "<<inBuffer_;
		if(connectionState_ == H_DISCONNECTING){
			inBuffer_.clear();
			break;
		}
		if(read_num < 0){
			perror("1");
			error_ = true;
			handleError(fd_, 400, "Bad request");
			break;
		}
		else if(zero){
			connectionState_ == H_DISCONNECTING;
			if(read_num == 0)
				break;
		}
		if(state_ == STATE_PARSE_URI){
			URIState flag = this->parse_URI();
			if(flag == PARSE_URI_AGAIN)
				break;
			else if(flag == PARSE_URI_ERROR){
				perror("2");
				LOG << "FD = "<<fd_<<","<<inBuffer_<<"******";
				inBuffer_.clear();
				error_ = true;
				handleError(fd_, 400, "Bad Request");
				break;
			}	
			else
				state_ = PARSE_URI_SUCCESS;
			
		}
		if(state_ == STATE_PARSE_HEADERS){
			HeaderState flag = this->parse_Headers();
			if(flag == PARSE_HEADER_AGAIN)
				break;
			else if(flag == PARSE_HEADER_ERROR){
				perror("3");
				error_ = true;
				handleError(fd_, 400, "Bad Request");
				break;
			}	
			if(method_ = METHOD_POST){
				state_ = STATE_RECV_BODY;
			}
			else{
				state_ = STATE_ANALYSIS;
			}
		}
		if(state_ == STATE_RECV_BODY){
			int content_length = -1;
			if(headers.find("Content-length")!=headers.end()){
				content_length = stoi(headers["Content-length"]);
			}
			else{
				error = true;
				handleError(fd, 400, "Bad Request: Lack of argument (Content-length)");
				break;
			}
			if(inBuffer_.size() < content_length)
				break;
			state = STATE_ANALYSIS;
		}
		if(state == STATE_ANALYSIS){
			AnalysisState flag = this->analysisRequest();
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
	if(!error_){
		if(outBuffer_.size() > 0){
			handleWrite();
		}
		if(!error_ && state_ == STATE_FINISH){
			this->reset();
			if(inBuffer_.size() > 0){
				if(connectionState_ != H_DISCONNECTING)
					handleRead();
			}
		}
		else if(!error_ && connectionState_ != H_DISCONNECTING){
			events_ |= EPOLLIN;
		}
	}
}

void HttpData::handlewrite(){
	if(!error_){
		if(writen(fd, outBuffer) < 0){
			perror("written");
			events_ = 0;
			error_ = true;
		}
		else if(outBuffer_.size() < 0)
			events_ |= EPOLLOUT;
	}
}

void HttpData::handleError(int fd, int err_num, std::string short_msg){
	short_msg = " " + short_msg;
	char send_buff[4096];
	string body_buff, head_buff;
	body_buff += "<html><title>something wrong</title></html>";
    	body_buff += "<body bgcolor=\"ffffff\">";
    	body_buff += to_string(err_num) + short_msg;
    	body_buff += "<hr><em> WebServer</em>\n</body></html>";

    	header_buff += "HTTP/1.1 " + to_string(err_num) + short_msg + "\r\n";
    	header_buff += "Content-Type: text/html\r\n";
    	header_buff += "Connection: Close\r\n";
    	header_buff += "Content-Length: " + to_string(body_buff.size()) + "\r\n";
    	header_buff += "Server: WebServer\r\n";;
    	header_buff += "\r\n";
}

void HttpData::handleConn(){
	seperateTimer();
	__uint32_t &events_ = channel_->getEvents();
	if(!error_ && connectionState_ == H_CONNECTED){
		if(events_ != 0){
			int timeout = DEFAULT_EXPIRED_TIME;
			if(keep_alive){
				timeout = DEFAULT_KEEP_ALIVE_TIME; DEFAULT_KEEP_ALIVE_TIME;				
			}
			if((events & EPOLLIN) && (events & EPOLLOUT)){
				events = __uint32_t(0);
				events |= EPOLLOUT;
			}
			events |= EPOLLET;
			loop_->updatePoller(channel_, timeout);
		}
		else if(keep_alive){
			events |= (EPOLLIN | EPOLLET | EPOLLONESHOT);
			int timeout = DEFAULT_EXPIRED_TIME;
			loop_->updatePoller(channel_, timeout);
		}
		else{
			events_ |= (EPOLLIN | EPOLLET);
			int timeout = (DEFAULT_KEEP_ALIVE_TIME >> 1);			
			loop_->updatePoller(channel_, timeout);
		}
	}
	else if(!error_ && connectionState_ == H_DISCONNECTING && (events_& EPOLLOUT)){
		events_ = (EPOLLOUT | EPOLLET);
	}
	else{
		loop_->runInLoop(bind(&HttpData::handleClose, shared_from_this()));
	}
}
