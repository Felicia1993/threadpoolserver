#pragma once

#include "CountDownLatch.h"
#include <memory>

#include <functional>
#include "noncopyable.h"
#include <memory>
#include <pthread.h>

class Thread : noncopyable
{
public:
  	typedef std::function<void ()> ThreadFunc;

  	explicit Thread(const ThreadFunc&, const string& name = string());
  	explicit Thread(ThreadFunc&&, const string& name = string());
  	~Thread();

  	void start();
  	int join(); // return pthread_join()

  	bool started() const { return started_; }
  // pthread_t pthreadId() const { return pthreadId_; }
  	pid_t tid() const { return *tid_; }
  	const string& name() const { return name_; }

  	static int numCreated() { return numCreated_.get(); }

private:
  	void setDefaultName();

  	bool       started_;
  	bool       joined_;
  	pthread_t  pthreadId_;
  	std::shared_ptr<pid_t> tid_;
  	ThreadFunc func_;
  	string     name_;
	CountDownLatch latch_;
};


