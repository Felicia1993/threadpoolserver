#include "TcpServer.h"

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr){
	
}

TcpServer::~TcpServer(){

}

void TcpServer::setThreadNum(){

}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr){
	
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn){

}

void removeConnectionInLoop(const TcpConnectionPtr& conn){

}
