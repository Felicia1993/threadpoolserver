#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <vector>
#include <memory>
#include <functional>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
#include <time.h>
#include <thread>

using namespace std;

#define gettid() syscall(SYS_gettid)

class EventLoop 
{
public:
	
  	EventLoop();
  	~EventLoop();  
  	void loop();
	void quit();
  	void assertInLoopThread(){
    		if (!isInLoopThread()){
      			abortNotInLoopThread();
    		}
  	}
  	bool isInLoopThread() const { return threadId_ == gettid(); }
//	string epollReturnTime() const{return epollReturnTime_;}
	
private:
//	typedef std::function<void()> Functor;
//	typedef std::vector<Channel*> ChannelList;
//	shared_ptr<EPollPoller> poller_;
//	ChannelList activeChannels_;
//	bool quit_;
	void abortNotInLoopThread();
	bool looping_;	
  	const pid_t threadId_;  
//	shared_ptr<Epoll> epoll_;
};
#endif
