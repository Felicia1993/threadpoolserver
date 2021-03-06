#pragma once
#include "requestData.h"
#include <pthread.h>
#include <functional>
#include <memory>
#include <vector>

const int THREADPOOL_INVALID = -1;
const int THREADPOOL_LOCK_FAILURE = -2;
const int THREADPOOL_QUEUE_FULL = -3;
const int THREADPOOL_SHUTDOWN = -4;
const int THREADPOOL_THREAD_FAILURE = -5;
const int THREADPOOL_GRACEFUL = 1;

const int MAX_THREADS = 1024;
const int MAX_QUEUE = 65535;

typedef enum{
	immediate_shutdown = 1,
	graceful_shutdown = 2
}threadpool_shutdown_t;

/*typedef struct{
	void(*function)(void *);
	void *argument;
}threadpool_task_t;*/

struct ThreadPoolTask{
	std::function<void(std::shared_ptr<void>)>fun;
	std::shared_ptr<void> args;
};

void myHandler(std::shared_ptr<void> req);
class ThreadPool{
private:
	static pthread_mutex_t lock;
	static pthread_cond_t notify;
	static std::vector<pthread_t> threads;
	static std::vector<ThreadPoolTask> queue;
	static int thread_count;
	static int queue_size;
	static int head;
	static int tail;
	static int count;
	static int shutdown;
	static int started;
public:
	static int threadpool_create(int thread_count, int queue_size);
	static int threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun = myHandler);
	static int threadpool_destroy();
	static int threadpool_free();
	static void *threadpool_thread(void *args);
};


