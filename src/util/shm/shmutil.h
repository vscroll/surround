#ifndef SHMUTIL_H
#define SHMUTIL_H

#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

class SHMUtil
{
public:
    SHMUtil(bool isNoWait = true);
    virtual ~SHMUtil();
    int create(key_t key, unsigned int size);
    void destroy();
    int write(unsigned char* buf, unsigned int size);
    int read(unsigned char* buf, unsigned int size);
    void* getSHMAddr() { return mSHMAddr; }
    int p_w();
    int v_w();
    int p_r();
    int v_r();
private:
    int sem_create(key_t key);
    int sem_del(int semid);
    int sem_set_value(int semid, int num, int val);
    int p(int semid, int num);
    int v(int semid, int num);

private:
    int mSemId;
    int mSHMId;
    void* mSHMAddr;
    bool mIsNoWait;
};

#endif // SHMUTIL_H
