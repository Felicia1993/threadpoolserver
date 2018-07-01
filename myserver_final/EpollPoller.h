#ifndef EPOLLER_H
#define EPOLLER_H

#include <map>
#include <vector>
#include <string.h>
#include <sys/epoll.h>
#include <iostream>

class Channel;
class EventLoop;

class EPollPoller 
{
public:
	friend class Channel;
	friend class EventLoop;
	EPollPoller(EventLoop* loop);
  	virtual ~EPollPoller();
  	typedef std::vector<Channel*> ChannelList;
  	
	virtual void updateChannel(Channel* channel);
	void update(int operation, Channel* channel);

//	Polls the I/O events
	virtual string poll(int timeoutMs, ChannelList *activeChannels);
  	void assertInLoopThread() const
  	{
    		ownerLoop_->assertInLoopThread();
  	}
	
	typedef std::vector<struct epoll_event> EventList;
	EventList events_;
	int epollfd_;

private:
	static const int kInitEventListSize = 16;	
	void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
	typedef std::map<int, Channel*> ChannelMap;
  	EventLoop* ownerLoop_;
	ChannelMap channels_;
};
#endif
