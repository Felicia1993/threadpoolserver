#include <iostream>
#include "EventLoop.h"
#include <thread>

using namespace std;

void threadFunc(){
	printf("threadFunc():pid = %d, tid = %d\n",getpid(), gettid());
	EventLoop loop;
	loop.loop();
}

int main(){
	printf("threadFunc():pid = %d, tid = %d\n",getpid(), gettid());
	EventLoop loop;
	thread t(threadFunc);
	t.join();
	loop.loop();
}
