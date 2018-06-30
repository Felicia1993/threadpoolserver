#include <iostream>
#include "EventLoop.h"
#include <thread>

using namespace std;

EventLoop* g_loop;

void threadFunc(){
	g_loop->loop();
}

int main(){
	EventLoop loop;
	g_loop = &loop;
	thread t(threadFunc);
	t.join();
}
