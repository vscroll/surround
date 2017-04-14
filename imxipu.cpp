#include "imxipu.h"
#include <linux/ipu.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <QDebug>

IMXIPU::IMXIPU()
{
}

int IMXIPU::allocIPUBuf(int fd, struct buffer* ipu_buf,  unsigned int frame_size)
{
    //unsigned int page_size = getpagesize();
    unsigned int buf_size = frame_size;
    // buf_size = (buf_size + page_size - 1) & ~(page_size - 1);
    ipu_buf->length = ipu_buf->offset = buf_size;
    if (-1 == ioctl(fd, IPU_ALLOC, &ipu_buf->offset))
    {
        qDebug() << "IMXIPU::initV4l2Buf"
                 << " IPU_ALLOC failed";
        return -1;
    }

    ipu_buf->start = mmap(0, buf_size, PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, ipu_buf->offset);
    if (NULL == ipu_buf->start)
    {
        qDebug() << "IMXIPU::initV4l2Buf"
                 << " ipu map failed";
        return -1;
    }
}

int IMXIPU::freeIPUBuf(int fd, struct buffer* ipu_buf)
{

}

int IMXIPU::IPUTask(int fd, struct ipu_task* task)
{
    return ioctl(fd, IPU_QUEUE_TASK, task);
}
