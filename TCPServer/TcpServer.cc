#include "TcpServer.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util.h"
#include "base/Logging.h"
#include <functional>

TcpServer::TcpServer(EventLoop* loop, int threadnum, int port):loop_(loop), threadNum_(threadnum), port_(port),started_(false),listenFd_(socket_bind_listen(port_)){
	acceptChannel_->setFd(listenFd_);
	handle_for_sigpipe();
	if(setSocketNonBlocking(listenFd_) < 0){
		printf("set non block error\n");
		abort();
	}
}

TcpServer::~TcpServer(){
	
}

void TcpServer::setThreadNum(int num){
	threadNum_ = num;
}

void TcpServer::handNewConn(){
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(struct sockaddr_in));
	socklen_t client_addr_len = sizeof(client_addr);
	int accept_fd = 0;
	while((accept_fd = accept(listenFd_, (struct sockaddr*)&client_addr, &client_addr_len)) > 0){
		EventLoop* loop = eventLoopThreadPool_->getNextLoop();
		LOG << "New connection from "<<inet_ntoa(client_addr.sin_addr) << ":"<<ntohs(client_addr.sin_port);
		if(accept_fd >= MAXFDS){
			close(accept_fd);
			continue;
		}
		if(setSocketNonBlocking(accept_fd) < 0){
			LOG << "Set non block failed.\n";
			return;
		}
		setSocketNodelay(accept_fd);
	
		shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
		req_info->getChannel()->setHolder(req_info);
		loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));
	}
	acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}
	
void TcpServer::start(){
	eventLoopThreadPool_->start();
	acceptChannel_->setEvents(EPOLLIN | EPOLLET);
	acceptChannel_->setReadCallback(bind(&TcpServer::handNewConn, this));
	acceptChannel_->setConnCallback(bind(&TcpServer::handThisConn, this));
	loop_->addToPoller(acceptChannel_, 0);
	started_ = true;
}

