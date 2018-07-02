#pragma once
#include <sys/epoll.h>
#include <memory>
#include <unordered_map>
#include <vector>
#include <map>
#include "Channel.h"
#include "EventLoop.h"

class Epoll{
//	typedef std::shared_ptr<requestData> SP_ReqData;

public:
	typedef std::vector<Channel*> ChannelList;
	Epoll(EventLoop* loop);
	~Epoll();
	void updateChannel(Channel* channel);
	void assertInLoopThread(){
		ownerloop_->assertInLoopThread();
	}
	int poll(int timeoutMs, ChannelList* activeChannels);
private:
	void fillActiveChannels(int numEvents, ChannelList* activeChannels)const;

	typedef std::vector<struct epoll_event>EventList;
	typedef std::map<int, Channel*> ChannelMap; 
	EventLoop* ownerloop_;
	int epollfd_;
	EventList events_;
	ChannelMap channels_;
};

