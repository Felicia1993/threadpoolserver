#include "LogFile.h"
#include <stdio.h>
#include <time.h>
#include <assert.h>

LogFile::LogFile(const string& basename, int flushEveryN = 1024):basename_(basename),flushEveryN(flushEveryN_),count_(0), mutex_(new MutexLock){
	file_.reset(new AppendFile(basename));
}

LogFile::~LogFile(){}

void LogFile::append(const char* logline, int len){
	MutexLockGuard lock(*mutex_);
	append_unlocked(logline, len);
}

void LogFile::flush(){
	MutexLockGuard lock(*mutex_);	
	file_->flush();
}


bool LogFile::append_unlocked{
	file_->append(logline, len);
	++count_;
	if(count_ >= flushEveryN_){
		count_ = 0;
		file_->flush();
	}
}

