#include "base/MutexLock.h"
#include "base/Thread.h"
#include "EventLoop.h"
#include "base/Condition.h"
#include "base/MutexLock.h"

class EventLoopThread{
public:
	EventLoopThread();
	~EventLoopThread();
	EventLoop* startLoop();
private:
	void threadFunc();
	EventLoop* loop_;
	bool existing_;
	Thread thread_;
	Condition cond_;
	MutexLock mutex_;
};
