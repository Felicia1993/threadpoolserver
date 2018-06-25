#pragma once
#include "FileUtil.h"
#include "./base/mutexLock.hpp"
#include "noncopyable.h"
#include <memory>
#include <string>

class LogFile:noncopyable{
public:
	LogFile(const string& basename, int flushEveryN = 1024);
	~LogFile();
	void append(const char* logline, int len);
	void flush();
	bool rollFile();
private:
	void append_unlocked(const char* logline, int len);
	const string basename_;
	const int flushEveryN_;
	int count_;
	std::unique_str<MutexLock> mutex_;
	std::unique_str<AppendFile> file_;
};

