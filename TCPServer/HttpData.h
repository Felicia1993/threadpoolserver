#pragma once
#include "Timer.h"
#include <string>
#include <unordered_map>
#include <map>
#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <unistd.h>

class EventLoop;
class TimerNode;
class Channel;

enum ProcessState{
	STATE_PARSE_URI = 1,
	STATE_PARSE_HEADERS,
	STATE_RECV_BODY,
	STATE_ANALYSIS,
	STATE_FINISH
};

enum URIState{
	PARSE_URI_AGAIN = 1,
	PARSE_URI_ERROR,
	PARSE_URI_SUCCESS
};

enum HeaderState{
	PARSE_HEADER_SUCCESS = 1,
	PARSE_HEADER_AGAIN,
	PARSE_HEADER_ERROR
};

enum AnalysisState{
	ANALYSIS_SUCCESS = 1,
	ANALYSIS_ERROR
};

enum ParseState{
	H_START = 0,	
	H_KEY,
	H_COLON,
	h_SPACE_AFTER_COLON,
	H_VALUE,
	H_CR,
	H_LF,
	H_END_CR,
	H_END_LF
};
