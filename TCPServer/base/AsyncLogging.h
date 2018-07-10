#pragma once
#include "LogStream.h"
#include "Condition.h"
#include "MutexLock.h"
#include <memory>
#include <vector>

class AsyncLogging{
public:
	AsyncLogging();
	~AsyncLogging();
	void start(){
		running_ = true;
		thread_.start();
		latch_.wait();
	}

	void stop(){
		running_ = false;
		cond_.notify();
		thread_.join()
	}	
	void append(const char* logline, int len);
	void threadFunc();
private:
	typedef FixedBuffer<kLargeBuffer> Buffer;
	typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
	typedef std::shared_ptr<Buffer> BufferPtr;
	bool running_;
	const int flushInterval_;
	Thread thread_;
	std::string basename_;
	BufferPtr currentBuffer_;
	BufferPtr nextBuffer_;
	BufferVector buffers_;
	MutexLock mutex_;
	Condition cond_;
	CountDownLatch latch_;
};
