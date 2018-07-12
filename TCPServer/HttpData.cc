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

}

void HttpData::handleClose(){

}

void HttpData::newEvent(){

}

HeaderState HttpData::parse_Headers(){

}

AnalysisState HttpData::analysisRequest(){

}

void HttpData::handleread(){

}

void HttpData::handlewrite(){

}

void HttpData::handleError(int fd, int err_num, std::string short_msg){

}

void HttpData::handleConn(){

}
