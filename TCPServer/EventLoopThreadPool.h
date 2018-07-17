#include "base/Thread.h"
#include "EventLoop.h"
#include "EventLoopThread.h"


class EventLoopThreadPool{
public:
	EventLoopThreadPool(EventLoop* baseLoop, int numThreads);
	~EventLoopThreadPool(){
		LOG << "~EventLoopThreadPool()";
	}
	void start();
	EventLoop* getNextLoop();
private:
	EventLoop* baseLoop_;
	bool started_;
	int numThreads_;
	int next_;
	vector<shared_ptr<EventLoopThread>> threads_;
	vector<EventLoop*> loops_;
};
