#include "EventLoop.h"
#include <iostream>
using namespace std;

EventLoop::EventLoop(){
	cout<<"EventLoop structed\n";
}
EventLoop::~EventLoop(){
	cout<<"EventLoop constructed\n";
}

void EventLoop::testEventLoop(){
	cout<<"Channel  use EvnetLoop  success!\n";
	channel_->testChannel();
	
}
