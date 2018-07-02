#include "epoll.h"
#include "assert.h"
#include <iostream>

Epoll::Epoll(EventLoop* loop):ownerloop_(loop){

}

Epoll::~Epoll(){

}

void Epoll::updateChannel(Channel* channel){
//	assertInLoopThread();
	cout<<"this is Epoll::updateChannel!\n";
	std::cout<<"fd = ";
	std::cout<<channel->fd()<<" events = "<<channel->events();
/*	if(channel->index() < 0){
		assert(channels_.find(channel->fd()) == channels_.end());
		struct epoll_event pev;
		pev.events = static_cast<uint32_t>(channel->events());
		pev.data.fd = channel->fd();
		events_.push_back(pev);
		int idx = static_cast<int>(events_.size())-1;
		channel->set_index(idx);
		channels_[pev.data.fd] = channel;
	}
	else{
		//update existing one
		assert(channels_.find(channel->fd()) != channels_.end());
		assert(channels_[channel->fd()] == channel);
		int idx = channel->index();
		assert(0 <= idx && idx < static_cast<int>(events_.size()));
		struct epoll_event& pfd = events_[idx];
		assert(pfd.data.fd == channel->fd() || pfd.data.fd == -1);
		pfd.events = static_cast<short>(channel->events());
		if(channel->isNoneEvent()){
			pfd.data.fd = -1;
		}		
	}*/
}

void Epoll::fillActiveChannels(int numEvents, ChannelList* activeChannels) const{
	assert(static_cast<size_t>(numEvents) <= events_.size());
	for(int i =0 ; i < numEvents; i++){
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

int Epoll::poll(int timeoutMs, ChannelList* activeChannels){
	int numEvents = epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
	if(numEvents > 0){
		std::cout<<numEvents<<" events happened";
		fillActiveChannels(numEvents, activeChannels);
		if(static_cast<size_t>(numEvents) == events_.size()){
			events_.resize(events_.size()*2);
		}
	}
	return numEvents;
}
