#ifndef EPOLLER_H
#define EPOLLER_H

#include <map>
#include <vector>
#include <string>
#include <sys/epoll.h>
#include <iostream>
class Channel;
class EventLoop;

class EPollPoller 
{
public:
	EPollPoller(EventLoop* loop);
  	virtual ~EPollPoller();
  	typedef std::vector<Channel*> ChannelList;
  	
	virtual void updateChannel(Channel* channel);
	void update(int operation, Channel* channel);

//	Polls the I/O events
	virtual char* poll(const int timeoutMs, ChannelList* activeChannels);
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
