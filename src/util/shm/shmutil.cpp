#include "shmutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#define SEM_N 2
#define W   1  //写操作的信号灯
#define R   0	//读操作的信号灯

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

SHMUtil::SHMUtil()
{
    mSHMId = -1;
    mSHMAddr = NULL;
    mSemId = -1;
}

SHMUtil::~SHMUtil()
{

}

int SHMUtil::create(key_t key, unsigned int size)
{
    mSemId = sem_create(key);
    if (mSemId < 0)
    {
        perror("sem_open");
        return -1;
    }

    if ((mSHMId = shmget(key, size, 0666 | IPC_CREAT)) == -1)
    {
        perror("shmget");
        return -1;
    }

    if ((mSHMAddr = shmat(mSHMId, NULL, 0)) == NULL)
    {
        perror("shmat");
        return -1;
    }

    return 0;
}

void SHMUtil::destroy()
{
    if (NULL != mSHMAddr)
    {
	    shmdt(mSHMAddr);
        mSHMAddr = NULL;
    }

    if (mSemId > 0)
    {
	    sem_del(mSemId);
	    mSemId = -1;
    }
}

int SHMUtil::write(unsigned char* buf, unsigned int size)
{
    if (NULL == mSHMAddr
	    || mSemId < 0)
    {
	    return -1;
    }

    p(mSemId, W);
    memcpy(mSHMAddr, buf, size);
    v(mSemId, R);

    return 0;
}

int SHMUtil::read(unsigned char* buf, unsigned int size)
{
    if (NULL == mSHMAddr
	    || mSemId < 0)
    {
	    return -1;
    }

    p(mSemId, R);
    memcpy(buf, mSHMAddr, size);
    v(mSemId, W);

    return 0;
}

int SHMUtil::sem_create(key_t key)
{
    int semid = semget(key, SEM_N, IPC_CREAT | 0666);
    if (semid < 0)
    {
	    perror("sem_create");
	    return -1;
    }

    sem_set_value(semid, W, 1);
    sem_set_value(semid, R, 0);
    
    return semid;
}

int SHMUtil::sem_del(int semid)
{
    union semun sem;
    sem.val = 0;
    if (semctl(semid, 0, IPC_RMID, sem) < 0)
    {
        perror("sem_del");
        return -1;
    }
    return 0;
}

int SHMUtil::sem_set_value(int semid, int num, int val) 
{
    union semun sem_un;
    sem_un.val = val;
    if (semctl(semid, num, SETVAL, sem_un) < 0)
    {
        perror("sem_set_value");
        return -1;
    }
    return 0;
}

int SHMUtil::p(int semid, int num)
{
    struct sembuf sem;
    sem.sem_num = num;
    sem.sem_op = -1;
    sem.sem_flg = 0;//IPC_NOWAIT;
    if (semop(semid, &sem, 1) < 0)
    {
        perror("semop");
	    return -1;
    }
    return 0;
}

int SHMUtil::v(int semid, int num)
{
    struct sembuf sem;
    sem.sem_num = num;
    sem.sem_op = +1;
    sem.sem_flg = 0;//IPC_NOWAIT;
    if (semop(semid, &sem, 1) < 0)
    {
        perror("semop");
	    return -1;
    }
    return 0;
}

int SHMUtil::p_w()
{
    return p(mSemId, W);
}

int SHMUtil::v_w()
{
    return v(mSemId, W);
}

int SHMUtil::p_r()
{
    return p(mSemId, R);
}

int SHMUtil::v_r()
{
    return v(mSemId, R);
}
