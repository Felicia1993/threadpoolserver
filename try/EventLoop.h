#include <vector>
#include <memory>
#include "Channel.h"

class EventLoop{
public:
	EventLoop();
	~EventLoop();
//	void printChannel(Channel* a);
	void testEventLoop();
private:
	shared_ptr<Channel*> channel_;
};
