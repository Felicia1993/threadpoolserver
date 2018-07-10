#include <memory>
#include "MutexLock.h"
#include <string>
#include "noncopyable.hpp"

class LogFile:noncopyable{
public:
	LogFile(const string& basename_, const int flushEveryN_ );
	~LogFile();
	void append(const char* logline, int len);
	void flush();
private:
	void append_unlocked(const char* logline, int len);

	const std::string basename_;
	const int flushEveryN_;
	
	int count_;
	std::unique_ptr<MutexLock> mutex_;
	std::unique_ptr<AppendFile> file_;
};
