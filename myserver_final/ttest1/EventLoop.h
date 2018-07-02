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
class Channel;
class Epoll;

class EventLoop 
{
public:
	friend class Channel;
	friend class Epoll;
  	EventLoop();
  	~EventLoop();  
  	void loop();
	void quit();
  	void assertInLoopThread(){
		cout<<"Epoll->EventLoop!\n";
    		if (!isInLoopThread()){
			cout<<"isInLoopThread()\n";
      			abortNotInLoopThread();
    		}
		cout<<"assertInLoopThread-finished\n";
  	}
  	bool isInLoopThread() const { return threadId_ == gettid(); }
	void updateChannel(Channel* channel);
	
private:
//	typedef std::function<void()> Functor;
	typedef std::vector<Channel*> ChannelList;

	shared_ptr<Epoll> poller_;
	ChannelList activeChannels_;
	bool quit_;
	void abortNotInLoopThread();
	bool looping_;	
  	const pid_t threadId_;  

};
#endif
