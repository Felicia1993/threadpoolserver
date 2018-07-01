#include "EventLoop.h"
#include "Channel.h"
#include <iostream>
using namespace std;

EventLoop::EventLoop(){
	cout<<"EventLoop structed\n";
}
EventLoop::~EventLoop(){
	cout<<"EventLoop constructed\n";
}

void EventLoop::testEventLoop(Channel* channel){
	cout<<"Channel  use EvnetLoop  success!\n";
	channel->testChannel();
	
}
