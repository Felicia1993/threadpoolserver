#include "EventLoop.h"

__thread EventLoop* t_loopInThisThread = 0;

int createEventfd(){
	int evtfd = eventfd(0, EFD_NONBLOCK | EFD_COLEXEC);
	if(evtfd < 0){
		LOG << "Failed in eventfd";
		abort();
	}
	return evtfd;
}
EventLoop::EventLoop():looping_(false),quit_(false),threadId_(CurrentThread::tid()),epoll_(new Epoll()),wakeupFd_(createEventfd()),pwakeupChannel_(new Channel(this, wakeupFd_)),eventHandling_(false),callingPendingFunctors_(false){
	if(t_loopInThread){
		LOG<<"Another EventLoop "<< t_loopInThisThread<<" exists in this thread" << threadId_;
	}
	else{
		t_loopInThisThread = this;
	}

	pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
	pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
	pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
	poller_->epoll_add(pwakeupChannel_, 0);
}

void EventLoop::handleConn(){
	updatePoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop(){
	close(wakeupFd_);
	t_loopInThisThread = NULL;
}

void EventLoop::wakeup(){
	uint64_t one = 1;
	ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
	if(n != one){
		LOG<<"EventLoop::wakeup() writes "<< n << "bytes instead of 8";
	}
}

void EventLoop::handleRead(){
	uint64_t one = 1;
	ssize_t n = readn(wakeupFd_, &one, sizeof one);
	if(n != sizeof one){
		LOG << "EventLoop::handleread() reads"<<n<<"bytes instead of 8";
	}
	pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::runInLoop(Functor&& cb){
	if(isInLoopThread()){
		cb();
	}
	else{
		queueInLoop(std::move(cb));
	}
}

void EventLoop::queueInLoop(Functor&& cb){
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.emplace_back(std::move(cb));
	}
	if(!isInLoopThread() || callingPendingFunctors_)
		wakeup();
}

void EventLoop::loop(){
	assert(!looping_);
	assert(isInLoopThread());
	looping_ = true;
	quit_ = false;
	
	std::vector<SP_Channel> ret;
	while(!quit_){
		ret.clear();
		ret = poller_->poll();
		eventHandling_ = true;
		for(auto &it : ret){
			it->handleEvents();
		}
		eventHandling_ = false;
		doPendingFunctors();
		poller_->handleExpired();
	}
	looping_ = false;
}
