#pragma once
#include <assert.h>
#include <string.h>
#include <string>

class AsyncLogging;
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000*1000;

template <int SIZE>
class FixedBuffer:noncopyable{
public:
	FixedBuffer(): cur_(data_){
    		
  	}
  	~FixedBuffer(){
    	
  	}
	void append(const char* buf, size_t len){
    		if (implicit_cast<size_t>(avail()) > len){
      			memcpy(cur_, buf, len);
      			cur_ += len;
    		}
  	}
	const char* data() const { return data_; }
  	int length() const { return static_cast<int>(cur_ - data_); }
	char* current() { return cur_; }
  	int avail() const { return static_cast<int>(end() - cur_); }
  	void add(size_t len) { cur_ += len; }
 	void reset() { cur_ = data_; }
  	void bzero() { ::bzero(data_, sizeof data_); }
	
private:
	const char* end() const{
		return data_ + sizeof data_; 
	}
	char data_[SIZE];
  	char* cur_;
};

class LogStream:noncopyable{
	typedef LogStream self;
public:
	typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

  	self& operator<<(bool v)
  	{
    		buffer_.append(v ? "1" : "0", 1);
    		return *this;
  	}

  	self& operator<<(short);
  	self& operator<<(unsigned short);
  	self& operator<<(int);
  	self& operator<<(unsigned int);
  	self& operator<<(long);
  	self& operator<<(unsigned long);
  	self& operator<<(long long);
  	self& operator<<(unsigned long long);

  	self& operator<<(const void*);

  	self& operator<<(float v){
    		*this << static_cast<double>(v);
    		return *this;
  	}
	self& operator<<(double);

  	self& operator<<(char v)
  	{
  	  	buffer_.append(&v, 1);
   	 	return *this;
  	}

  	self& operator<<(const char* str){
    		if (str){
      			buffer_.append(str, strlen(str));
    		}
    		else{
      			buffer_.append("(null)", 6);
    		}
    		return *this;
  	}
	void append(const char* data, int len) { buffer_.append(data, len); }
  	const Buffer& buffer() const { return buffer_; }
  	void resetBuffer() { buffer_.reset(); }
private:
  	void staticCheck();
  	template<typename T>
  	void formatInteger(T);
  	Buffer buffer_;
  	static const int kMaxNumericSize = 32;
};

