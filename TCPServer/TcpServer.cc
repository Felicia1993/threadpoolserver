#include "TcpServer.h"

TcpServer::TcpServer(EventLoop* loop, int threadnum, int port):loop_(loop), threadNum_(threadnum), port_(port),started_(false),listenFd_(socket_bind_listen(port_)){
	acceptChannel_->
}

TcpServer::~TcpServer(){
	
}

void TcpServer::setThreadNum(int num){

}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr){
	
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn){

}

void removeConnectionInLoop(const TcpConnectionPtr& conn){

}
