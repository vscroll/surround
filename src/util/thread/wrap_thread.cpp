#include "wrap_thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sys/syscall.h>

void* thread_func(void* args)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    WrapThread* pThread = static_cast<WrapThread*>(args);

    if (pThread->mBindCPUNo >= 0)
    {
        pThread->bindCPU(pThread->mBindCPUNo);
    }

    std::cout << "Thread id:" << pThread->getTID()
              << " BindCPUNo:" << pThread->mBindCPUNo
              << " interval:" << pThread->mInterval
              << std::endl;

    clock_t lastTime = 0;
    double elapsed = 0.0;
    while(true)
    {
	    pthread_testcancel();

        if (lastTime != 0)
        {
            elapsed = (double)(clock() - lastTime)/CLOCKS_PER_SEC;
        }

	    if (elapsed > 0.00001f
		    && elapsed*1000 < pThread->mInterval)
	    {
	        continue;
	    }

        lastTime = clock();

        pThread->run();
        usleep(5);
	    //usleep(pThread->mInterval);
    }
    return NULL;   
}

WrapThread::WrapThread()
{
    mThreadId = 0;
    mInterval = 10;
    mBindCPUNo = -1;
}

WrapThread::~WrapThread()
{

}

bool WrapThread::start(unsigned int interval, int bindCPUNo)
{
    mInterval = interval;
    mBindCPUNo = bindCPUNo;
    return pthread_create(&mThreadId, NULL, thread_func, this) == 0;
}

void WrapThread::stop()
{
    pthread_cancel(mThreadId);
    pthread_join(mThreadId, NULL);
}

pthread_t WrapThread::getThreadID()
{
    return mThreadId;
}

long int WrapThread::getTID()
{
    return syscall(__NR_gettid);
}

int WrapThread::getCPUNumber()
{
    return sysconf(_SC_NPROCESSORS_CONF);
}

void WrapThread::bindCPU(int cpuNo)
{
    cpu_set_t mask;  
    CPU_ZERO(&mask);  
  
    CPU_SET(cpuNo, &mask);  
  
    std::cout << std::endl
              << "thread id:" << getTID()
              << ", CPU number:" << getCPUNumber()
              << ", cpuNo:" << cpuNo
              << std::endl; 
    pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
}
