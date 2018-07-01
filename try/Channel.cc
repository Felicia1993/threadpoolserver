#include "Channel.h"
#include "EventLoop.h"
#include <iostream>
using namespace std;

Channel::Channel(){
	cout<<"Channel structed\n";
}
Channel::~Channel(){
	cout<<"Channel constructed\n";
}

void Channel::testChannel(EventLoop* event){
	cout<<"EvnetLoop use Channel success!\n";
	event->
}
