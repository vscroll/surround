#include "thread.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sys/syscall.h>

void* thread_func(void* args)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    Thread* pThread = static_cast<Thread*>(args);

    if (pThread->mBindCPUNo >= 0)
    {
        pThread->bindCPU(pThread->mBindCPUNo);
    }

    std::cout << "Thread id:" << pThread->getTID()
              << " BindCPUNo:" << pThread->mBindCPUNo
              << " interval:" << pThread->mInterval
              << std::endl;

    double lastTimestamp = 0.0;
    double elapsed = 0.0;
    while(true)
    {
	pthread_testcancel();

        if (lastTimestamp > 0.00001f)
        {
            elapsed = (clock() - lastTimestamp)/CLOCKS_PER_SEC;
        }

	if (elapsed > 0.00001f
		&& elapsed*1000 < pThread->mInterval)
	{
	    continue;
	}

	lastTimestamp = clock();

	pThread->run();
	usleep(5);
	//usleep(pThread->mInterval);
    }
    return NULL;   
}

Thread::Thread()
{
    mThreadId = 0;
    mInterval = 10;
    mBindCPUNo = -1;
}

Thread::~Thread()
{

}

bool Thread::start(unsigned int interval, int bindCPUNo)
{
    mInterval = interval;
    mBindCPUNo = bindCPUNo;
    return pthread_create(&mThreadId, NULL, thread_func, this) == 0;
}

void Thread::stop()
{
    pthread_cancel(mThreadId);
    pthread_join(mThreadId, NULL);
}

pthread_t Thread::getThreadID()
{
    return mThreadId;
}

long int Thread::getTID()
{
    return syscall(__NR_gettid);
}

int Thread::getCPUNumber()
{
    return sysconf(_SC_NPROCESSORS_CONF);
}

void Thread::bindCPU(int cpuNo)
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
