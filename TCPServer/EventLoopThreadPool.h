#include "Thread.h"
#include "EventLoop.h"

class EventLoopThread{
public:
	EventLoopThread();
	~EventLoopThread();
	void setThreadNum(int numThreads){
		numThreads_ = numThreads;
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
