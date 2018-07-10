#include "AsyncLoggin.h"


void AsyncLogging::append(const char* logline, int len){
	MutexLockGuard(&mutex);
	if(currentBuffer_.avail() > len){
		currentBuffer_->append(logline, len);
	}
	else{
		buffers_.push_back(currenBuffer_);
		currentBuffer_.reset();
		if(nextBuffer_){
			currentBuffer_ = std::move(nextBuffer_);
		}
		else{
			currentBuffer_.reset(new Buffer);
		}
		currentBuffer_->append(logline, len);
		cond_.notify();
	}	
}

void AsynLogging::threadFunc(){
	assert(running_ == true);
	latch_.countDown();
	LogFile ouput(basename_);
	BufferPtr newBuffer1(new Buffer);
	newBuffer1->bzero();
	BufferPtr newBuffer2(new Buffer);
	newBuffer2->bzero();
	BufferVector bufferToWrite;
	bufferToWrite.reserve(16);
	while(running_){
		assert(newBuffer1 && newBuffer1->length() == 0);
		assert(newBuffer2 && newBuffer2->length() == 0);
		assert(bufferToWrite.empty());
		{
			MutexLockGuard lock(mutex_);	
			if(buffers_.empty()){
				cond_.waitForSecnds(flushInterval_);
			}	
			buffers_.push_back(currentBuffer_);
			currentBuffer_.reset();
			currentBuffer_ = std::move(newbuffer1);
			buffersToWrite.swap(buffers_);
			if(!nextBuffer_){
				nextBuffer_ = std::move(newBuffer2);
			}	
		}
		assert(!buffersToWrite.empty());
		if(buffersToWrite.size() > 25){
			buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
		}	
		for(size_t i = 0; i < buffersToWrite.size(); ++i){
			ouput.append(buffersToWrite[i]->data(),buffersToWrite[i]->length());
		}
		if(buffersToWrite.size() > 2){
			buffersToWrite.resize(2);
		}
		if(!newBuffer1){
			assert(!buffersToWrite.empty());
			newBuffer1 = buffersToWrite.back();
			buffersToWrite.pop_back();
			newBuffer1->reset();
		}			
		if(!newBuffer2){
			assert(!buffersToWrite.empty());
			newBuffer2 = buffersToWrite.back();
			buffersToWrite.pop_back();
			newBuffer2->reset();
		}
		buffersToWrite.clear();
		ouput.flush();
	}
	output.flush();
}
