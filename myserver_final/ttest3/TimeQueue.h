#include <queue>
#include <deque>
#include <algorithm>

class EventLoop;
class Channel;
class TimerQueue{
public:
	friend class EventLoop;
	friend class Channel;
	TimerQueue(EventLoop* loop);
	~TimerQueue();
private:
	typedef std::pair<Timestamp,Timer*> Entry;
	typedef std::pair<Timer*, int64_t> ActiveTimer;
	typedef std::priority_queue<Entry, deque<Entry>, cmp> TimerList;

	void handleRead();
	
	std::vector<Entry> getExpired(Timestamp now);
	void reset(const std::vector<Entry>& expired, Timestamp now);
	bool insert(Timer* timer);
	EventLoop* loop_;
	const int timerfd_;
	Channel timerfdChannel_;
	TimerList timers_;
	bool callingExpiredTimers_;
};
