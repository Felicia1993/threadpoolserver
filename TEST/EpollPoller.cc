#include "EpollPoller.h"
#define NDEBUG
#include <assert.h>
#include <time.h>
#include <iostream>
using namespace std;

namespace{
	const int kNew = -1;
	const int kAdded = 1;
	const int kDeleted = 2;
}



EPollPoller::EPollPoller(EventLoop* loop):ownerLoop_(loop){
	
}

EPollPoller::~EPollPoller(){

}

Timestamp EPollPoller::EpollPoller(int timeoutMs, ChannelList* activeChannels){
//int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout);	
//int poll(struct pollfd* fds, nfds_t nfds, int timeout);
	int numEvents = epoll(epollfd_, kInitEventListSize, timeoutMs);
//	Timestamp now(Timestamp::now());
	if(numEvents > 0){
		cout<< numEvents<<" events happened.\n";
	}
	else if(numEvents == 0){
		cout<<" nothing happened.\n";
	}
	else{
		cout<<"Poller::poll()";
	}
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels)const{
	assert(implicit_cast<size_t>(numEvents) <= events_.size());
	for(int i = 0; i < numEvents; i++){
		Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
		int fd = channel->fd();
		ChannelMap::const_iterator it = channels_.find(fd);
		assert(it != channels_.end());
		assert(it->second == channel);
#endif
		channel->set_revents(events_[i].events);
		activeChannels->push_back(channel);
	}
}

void EPollPoller::updateChannel( Channel* channel){
	assertInLoopThread();
	cout<<"fd = "<<channel->fd()<<" events = "<< channel->events()<<" index = "<<index;
	if(index == kNew || index == kDeleted){
		int fd = channel->fd();
		if( index == kNew){
			assert(channels_.find(fd) == channels_.end());
			channels_[fd] = channel;
		}
		else{//index = kDeleted
			assert(channels_.find(fd) != channels_.end());
			assert(channels_[fd] == channel);
		}
		channel->set_index(kAdded);
		update(EPOLL_CTL_ADD, channel);
	}
	else{
		//update existing one with EPOLL_CTL_MOD/DEL
		int fd = channel->fd();
		assert(channels_.find() != channels_.end());
		assert(channels[fd] == channel);
		assert(index == kAdded);		
		if(channel->isNoneEvent()){
			update(EPOLL_CTL_DEL, channel);
			channel->set_index(kDeleted);			
		}
		else{
			update(EPOLL_CTL_MOD, channel);
		}
	}
}

void EPollPoller::update(int operation, Channel* channel){
	struct epoll_event event;
	bzero(&event, sizeof event);
	event.events = channel->events();
	event.data.ptr = channel;
	int fd = channel->fd();
	if(epoll_ctl(epollfd_, operation, fd, &event) < 0){
		if(operation == EPOLL_CTL_DEL){
			cout<<"EPOLL_CTL_DEL"<<" fd = "<<fd<<endl;
		}
		else{
			cout<<"EPOLL_CTL_DEL"<<" fd = "<<fd<<endl;
		}
	}
}

char* EPollPoller::poll(int timeoutMs, ChannelList* activeChannels){
	cout << "fd total count " << channels_.size();
  	int numEvents = epoll_wait(epollfd_,
                               &*events_.begin(),
                               static_cast<int>(events_.size()),
                               timeoutMs);
  	int savedErrno = errno;
  //	Timestamp now(Timestamp::now());
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S", localtime(&timep));
  	if (numEvents > 0){
    		cout << numEvents << " events happened";
    		fillActiveChannels(numEvents, activeChannels);
    		if (implicit_cast<size_t>(numEvents) == events_.size()){
      			events_.resize(events_.size()*2);
    		}
  	}
  	else if (numEvents == 0){
    		cout << "nothing happened";
  	}
  	else{
    // error happens, log uncommon ones
    		if (savedErrno != EINTR){
      			errno = savedErrno;
      			cout << "EPollPoller::poll()";
    		}
  	}
  	return tmp;
}
