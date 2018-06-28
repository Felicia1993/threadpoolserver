#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <map>
#include <vector>
#include "EventLoop.h"

class Channel;

class EPollPoller 
{
public:
  	typedef std::vector<Channel*> ChannelList;

  	EpollPoller(EventLoop* loop);
  	virtual ~EpollPoller();

//	Polls the I/O events
	Timestamp poll(int timeoutMs, ChannelList* activeChannels);
  	virtual void updateChannel(Channel* channel) = 0;//pure virtual

  	void assertInLoopThread() const
  	{
    		ownerLoop_->assertInLoopThread();
  	}

private:
	static const int kInitEventListSize = 16;
	void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
	typedef std::vector<struct epoll_event> EventList;
	typedef std::map<int, Channel*> ChannelMap;
  	EventLoop* ownerLoop_;
	EventList events_;
	int epollfd_;
};
#endif  // MUDUO_NET_POLLER_H
