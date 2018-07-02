#pragma once
#include "requestData.h"
#include "./base/noncopyable.hpp"
#include "./base/mutexLock.hpp"
#include <unistd.h>
#include <memory>
#include <queue>
#include <deque>

class RequestData;

class TimeNode{
	typedef std::shared_ptr<RequestData> SP_ReqData;
private:
	bool deleted;
	size_t expired_time;
	SP_ReqData request_data;
public:
	TimeNode(SP_ReqData _request_data, int timeout);
	~TimeNode();
	void update(int timeout);
	bool isvalid();
	void clearReq();
	void setDeleted();
	bool isDeleted() const;
	size_t getExpTime() const;	
};

struct timerCmp{
	bool operator()(std::shared_ptr<TimeNode> &a, std::shared_ptr<TimeNode> &b) const{
		return a->getExpTime() > b->getExpTime();
	}
};

class TimerManager{
	typedef std::shared_ptr<RequestData> SP_ReqData;
	typedef std::shared_ptr<TimeNode> SP_TimeNode;
private:
	std::priority_queue<SP_TimeNode, std::deque<SP_TimeNode>, timerCmp> TimerNodeQueue;
	MutexLock lock;
public:
	TimerManager();
	~TimerManager();
	void addTimer(SP_ReqData request_data, int timeout);
	void addTimer(SP_TimeNode timer_node);
	void handle_expired_event();
};
