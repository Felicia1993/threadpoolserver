
class TcpServer:noncopyable{
public:
	TcpServer(EventLoop* loop, const InetAddress& listenAddr);
	~TcpServer();
	void setThreadNum();
private:
	void newConnection(int sockfd, const InetAddress& peerAddr);
	void removeConnection(const TcpConnectionPtr& conn);
	void removeConnectionInLoop(const TcpConnectionPtr& conn);

	typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

	EventLoop* loop_;
	const std::string name_;
	std::unique_str<Acceptor> acceptor_;
	std::unique_str<EventLoopThreadPool> threadPool_;	
};
