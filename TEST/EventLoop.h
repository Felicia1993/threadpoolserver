#pragma once

#include <vector>
#include <memory>
#include <vector>
#include <functional>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>
using namespace std;

#define gettid() syscall(SYS_gettid)

class EventLoop 
{
public:
  	typedef std::function<void()> Functor;
  	EventLoop();
  	~EventLoop();  
  	void loop();
  	void assertInLoopThread(){
    		if (!isInLoopThread()){
      			abortNotInLoopThread();
    		}
  	}

  	bool isInLoopThread() const { return threadId_ == gettid(); }

private:
	void abortNotInLoopThread();
	bool looping_;	
  	const pid_t threadId_;  
};


