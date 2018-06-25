#include "LogStream.h"

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
const char digitsHex[] = "0123456789ABCDEF";

template<typename T>
size_t convert(char buf[], T value){
  	T i = value;
  	char* p = buf;
  	do{
    		int lsd = static_cast<int>(i % 10);
    		i /= 10;
    		*p++ = zero[lsd];
  	} while (i != 0);
  	if (value < 0){
   		 *p++ = '-';
  	}
  	*p = '\0';
  	std::reverse(buf, p);
  	return p - buf;
}

template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

template<int SIZE>
const char* FixedBuffer<SIZE>::debugString()
{
  	*cur_ = '\0';
  	return data_;
}

template<typename T>
void LogStream::formatInteger(T v)
{
  	if (buffer_.avail() >= kMaxNumericSize){
    		size_t len = convert(buffer_.current(), v);
    		buffer_.add(len);
  	}
}

LogStream& LogStream::operator<<(short v)
{
  	*this << static_cast<int>(v);
  	return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
  	*this << static_cast<unsigned int>(v);
  	return *this;
}

LogStream& LogStream::operator<<(int v)
{
  	formatInteger(v);
  	return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
  	formatInteger(v);
  	return *this;
}

LogStream& LogStream::operator<<(long v)
{
  	formatInteger(v);
  	return *this;
}
