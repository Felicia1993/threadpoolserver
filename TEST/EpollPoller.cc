#include "Poller.h"

EPollPoller::Poller(EventLoop* loop):ownerLoop_(loop){
	
}

Poller::~Poller(){}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels){
//int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);	
//int poll(struct pollfd* fds, nfds_t nfds, int timeout);
	int numEvents = epoll()
}
