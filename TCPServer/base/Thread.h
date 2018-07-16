#pragma once

#include "CountDownLatch.h"
#include <memory>
#include <string>
#include <functional>
#include "noncopyable.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

class Thread : noncopyable
{
public:
  	typedef std::function<void ()> ThreadFunc;

  	explicit Thread(const ThreadFunc&, const std::string& name = std::string());
  	explicit Thread(ThreadFunc&&, const std::string& name = std::string());
  	~Thread();

  	void start();
  	int join(); // return pthread_join()

  	bool started() const { return started_; }
  	pid_t tid() const { return *tid_; }
  	const std::string& name() const { return name_; }


private:
  	void setDefaultName();

  	bool       started_;
  	bool       joined_;
  	pthread_t  pthreadId_;
  	std::shared_ptr<pid_t> tid_;
  	ThreadFunc func_;
  	std::string     name_;
	
	CountDownLatch latch_;
};


