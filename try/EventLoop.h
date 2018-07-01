#include <vector>
#include <memory>

class Channel;
class EventLoop{
public:
	friend class Channel;
	EventLoop();
	~EventLoop();
	void printChannel();
	void testEventLoop(Channel* channel);

};
