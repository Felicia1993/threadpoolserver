#include "Thread.h"
#include "EventLoop.h"

class EventLoopThread{
public:
	EventLoopThread(EventLoop* baseLoop, int numThreads);
	~EventLoopThread(){
		LOG << "~EventLoopThreadPool()";
	}
	void start();
	EventLoop* getNextLoop();
private:
	EventLoop* baseLoop_;
	bool started_;
	int numThreads_;
	int next_;
	Thread threads_;
	vector<EventLoop*> loops_;
};
