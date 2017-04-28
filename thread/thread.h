#ifndef THREAD_20170427_H  
#define THREAD_20170427_H
 
#include <pthread.h>
    
class Thread
{
public:
    Thread();
    virtual ~Thread();
    virtual void run() = 0;

    bool start(unsigned int interval);
    void stop();
    pthread_t getThreadID();

private:
    pthread_t mThreadId;
    unsigned int mInterval;
    //static void* thread_func(void * args);
    friend void* thread_func(void * args);
}; 

#endif
