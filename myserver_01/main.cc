#include "requestData.h"
#include "epoll.h"
#include "threadpool.h"
#include "util.h"
#include <sys/epoll.h>
#include <queue>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <unistd.h>

using namespace std;

const int THREADPOOL_THREAD_NUM = 4;
const int QUEUE_SIZE = 65535;

const int PORT = 8888;
const int ASK_STATIC_FILE = 1;
const int ASK_IMAGE_STITCH = 2;

const string PATH = "/";

static const int MAXEVENTS = 5000;
static const int LISTENQ = 1024;
const int TIMER_TIME_OUT = 500;


void acceptConnection(int listen_fd, int epoll_fd, const string &path);

extern priority_queue<mytimer*, deque<mytimer*>, timerCmp> myTimerQueue;

int socket_bind_listen(int port){
	if(port < 1024 || port > 65535)
		return -1;
	int listen_fd = 0;
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return -1;
	
	int optval = 1;
	if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		return -1;

	struct sockaddr_in server_addr;
	bzero((char*)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);
	if(bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
		return -1;
	if(listen(listen_fd, LISTENQ) == -1)
		return -1;

	if(listen_fd == -1){
		close(listen_fd);
		return -1;
	}
	return listen_fd;
}


void handle_expired_event(){
	MutexLockGuard lock;
	while(!myTimerQueue.empty()){		
		shared_ptr<mytimer> ptimer_now = myTimerQueue.top();
		if(ptimer_now->isDeleted()){
			myTimerQueue.pop();
		}
		else if(ptimer_now->isvalid() == false){
			myTimerQueue.pop();
		}
		else{
			break;
		}
	}

}

int main(){
	handle_for_sigpipe();
	;
	if(Epoll::epoll_init(MAXEVENTS, LISTENQ) < 0){
		perror("epoll init failed.");
		return 1;
	}
	if(ThreadPool::threadpool_create(THREADPOOL_THREAD_NUM, QUEUE_SIZE) < 0){
		printf("threadpool create failed.\n");
		return 1;
	}
	int listen_fd = socket_bind_listen(PORT);
	if(listen_fd < 0){
		perror("socket bind failed.");
		return 1;
	}
	if(setSocketNonBlocking(listen_fd) < 0){
		perror("set socket non block failed.");
		return 1;
	}

	__uint32_t event = EPOLLIN | EPOLLET;
	shared_ptr<requestData> req(new requestData());
	req->setFd(listen_fd);
	if(Epoll::epoll_add(listen_fd, req, event) < 0){
		printf("epoll add error!\n");
		return 1;
	}
		
	while(true){
		printf("hello this is while!\n");
		Epoll::my_epoll_wait(listen_fd, MAXEVENTS, -1);
		handle_expired_event();
	}
	return 0;
}
