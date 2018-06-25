#pragma once
#include "noncopyable.hpp"
#include <pthread.h>
#include <cstdio>

class MutexLock: noncopyable{
public:
	MutexLock(){
		pthread_mutex_init(&mutex, NULL);	
	}
	~MutexLock(){
		pthread_mutex_lock(&mutex);
		pthread_mutex_destroy(&mutex);
	}
	void lock(){
		pthread_mutex_lock(&mutex);
	}
	void unlock(){
		pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_t *get(){
		return &mutex;
	}
private:
	pthread_mutex_t mutex;

private:
	friend class Condition;
};

/*
class MutexLock:boost::noncopyable{
public:
	MutexLock():holder_(0){
		pthread_mutex_init(&mutex_, NULL);
	}
	~MutexLock(){
		assert(holder_ == 0);
		pthread_mutex_destroy(&mutex_);
	}

	bool isLockedByThisThread(){
		return holder_ == CurrentThread::tid();
	}

	void assertLocked(){
		assert(isLockedByThisThread());
	}
	
	void lock(){
		pthread_mtuex_lock(&mutex_);
		holder_ = CurrentThread::tid();
	}
	
	void unlock(){
		holder_ = 0;
		pthread_mutex_unlock(&mutex_);
	}
	
	pthread_mutex_t* getPthreadMutex(){
		return &mutex_;
	}
private:
	pthread_mutex_t mutex_;
	pid_t holder_;
};

class MutexLockGuard : boost::noncopyable{
public:
	explicit MutexLockGuard(MutexLock& mutex):mutex_(mutex){
		mutex_.lock();	
	}
	~MutexLockGuard(){
		mutex_.unlock();
	}
private:
	MutexLock& mutex_;
};
#define MutexLockGuard(x) static_assert(false, "missing mutex guard var name")
*/
class MutexLockGuard: noncopyable{
public:
	explicit MutexLockGuard(MutexLock &_mutex):
	mutex(_mutex){
		mutex.lock();
	}
	~MutexLockGuard(){
		mutex.unlock();
	}
private:
	MutexLock &mutex;
};
