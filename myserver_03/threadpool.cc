#include "threadpool.h"

pthread_mutex_t ThreadPool::lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::notify = PTHREAD_COND_INITIALIZER;
std::vector<phread_t> ThreadPool::threads;
std::vector<ThreadPoolTask> ThreadPool::queue;
int ThreadPool::thread_count = 0;
int ThreadPool::queue_size = 0;
int ThreadPool::head = 0;
int ThreadPool::tail = 0;
int ThreadPool::count = 0;
int ThreadPool::shutdown = 0;
int ThreadPool::started = 0;

int ThreadPool::threadpool_create(int _thread_count, int _queue_size){
	bool err = false;
	do{
		if(_thread_count <= 0 || _thread_count > MAX_THREADS || _queue_size < 0 || _queue_size > MAX_QUEUE){
			_thread_count = 4;	
			_queue_size = 1024;	
		}
		
		thread_count = 0;
		queue_size = _queue_size;
		head = tail = count = 0;
		shutdown = started = 0;
		
		threads.resize(_thread_count);
		queue.resize(_queue_size);

		for(int i = 0; i < _thread_count; i++){
			if(pthread_create(&threads[i], NULL, threadpool_thread, (void*)(0))!=0){
				return -1;
			}
			++thread_count;
			++started;
		}
	}while(false);
	
	if(err){
		return -1;	
	}
	return 0;
}

void myHandler(std::shared_ptr<void> req){
	std::shared_ptr<requestData> request = std::static_pointer_cast<requestData>(req);
	request->handleRequest();
}

int ThreadPool::threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun){
	int err = 0;
	int next;

	if(pthread_mutex_lock(&(pool->lock))!= 0){
		return THREADPOOL_LOCK_FAILURE;
	}
	
	do{
		next = (tail + 1) % queue_size;
		if(count == queue_size){
			err = THREADPOOL_QUEUE_FULL;
			break;
		}
		if(shutdown){
			err = THREADPOOL_SHUTDOWN;
			break;
		}
		queue[pool->tail].fun = fun;
		queue[pool->tail].args = args;
		tail = next;
		count += 1;

		if(pthread_cond_signal(&(pool->notify))!=0){
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}
	}while(false);

	if(pthread_mutex_unlock(&pool->lock) != 0){
		err = THREADPOOL_LOCK_FAILURE;
	}
	return err;
}

int ThreadPool::threadpool_destroy(ShutDownOption shutdown_option){
	printf("Threadpool destroy\n");
	int i, err = 0;

	if(pthread_mutex_lock(&(pool->lock))!= 0){
		return THREADPOOL_LOCK_FAILURE;
	}
	do{
		if(shutdown){
			err = THREADPOOL_SHUTDOWN;
			break;		
		}	
		shutdown = shutdown_option;
		if((pthread_cond_broadcast(&notify) !=0)|| (pthread_mutex_unlock(&lock)!=0)){
			err = THREADPOOL_LOCK_FAILURE;
			break;
		}

		for(i = 0; i < pool->thread_count; ++i){
			if(pthread_join(threads[i], NULL) != 0){
				err = THREADPOOL_THREAD_FAILURE;
				break;
			}
		}
	}while(false);

	if(!err){
		threadpool_free();
	}
	return err;
}

int threadpool_free(){
	if(started > 0){
		return -1;
	}
	pthread_mutex_lock(&(pool->lock));
	pthread_mutex_destroy(&(pool->lock));
	pthread_cond_destroy(&(pool->notify));
	return 0;
}

void *ThreadPool::threadpool_thread(void *args){
	while(true){
		ThreadPoolTask task;
		pthread_mutex_lock(&lock);
		while((count == 0) && (!shutdown)){
			pthread_cond_wait(&notify, &lock);
		}	
		
		if((shutdown == immediate_shutdown) || ((shutdown == graceful_shutdown) &&(count == 0))){
			break;
		}
		task.fun = queue[head].fun;
		task.args = queue[head].args;
		queue[head].fun = NULL;
		queue[head].args.reset();
		head = (head + 1) % queue_size;
		count -= 1;

		pthread_mutex_unlock(&lock);
		(task.fun)(task.args);
	}
	--started;
	pthread_mutex_unlock(&lock);
	printf("this threadpool thread finishs!\n");
	pthread_exit(NULL);
	return NULL;
}
