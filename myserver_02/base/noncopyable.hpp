#pragma once
class noncopyable{
protected:
	noncopyable() {}
	~noncopyalbe() {}
private:
	noncopyable(const noncopyable&);
	const noncopyable& operator=(const noncopyable&);
};
