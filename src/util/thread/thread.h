#ifndef THREAD_20170427_H  
#define THREAD_20170427_H
 
#include <pthread.h>
    
class Thread
{
public:
    Thread();
    virtual ~Thread();
    virtual void run() = 0;

    bool start(unsigned int interval, int bindCPUNo = -1);
    void stop();
    pthread_t getThreadID();
    long int getTID();
    static int getCPUNumber();
    unsigned int getInterval() { return mInterval; }
private:
    void bindCPU(int cpuNo);

    pthread_t mThreadId;
    unsigned int mInterval;
    int mBindCPUNo;
    //static void* thread_func(void * args);
    friend void* thread_func(void * args);
}; 

#endif
