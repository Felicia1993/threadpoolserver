#include <iostream>
#include "EventLoop.h"
#include "Channel.h"
using namespace std;

int main(){
	EventLoop event;	
	event->testEventLoop(&event);
	Channel ch;
	ch->testChannel(&ch);
}
