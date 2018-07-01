class EventLoop;
class Channel{
public:
	friend class EventLoop;
	Channel();
	~Channel();
	void testChannel(EventLoop* event);

};
