#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <errno.h>

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

//arm-linux-gcc v4l2_example.c -o v4l2_example.out -lopencv_core -lopencv_highgui
//arm-linux-gcc v4l2_example.c -o v4l2_example.out
//gcc v4l2_example.c -o v4l2_example.out

struct buffer
{
    void* start;
    unsigned int length;
};

struct buffer *gV4l2Buf = NULL;
static const  int V4L2_BUF_COUNT = 1;
static int WIDTH = 704;
static int HEIGHT = 574;

static void getVideoCap(int fd)
{
    struct v4l2_capability cap;
    if (-1 != ioctl(fd, VIDIOC_QUERYCAP, &cap))
    {
        printf("getVideoCap");
        printf(" driver:%d", cap.driver);
        printf(", card:%d", cap.card);
        printf(", version:%d", cap.version);
        printf("\n");
    }
}

static void getVideoFmt(int fd, int* width, int* height)
{
    struct v4l2_fmtdesc fmtDes;
    memset(&fmtDes, 0, sizeof(fmtDes));
    fmtDes.index = 0;
    fmtDes.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtDes) != -1)
    {
        fmtDes.index++;
        printf("getVideoFmt");
        printf(" support format:%d", fmtDes.index);
        printf(" format:%d", fmtDes.pixelformat);
        printf(" format desciption:%s", (char*)(fmtDes.description));
        printf("\n");
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 != ioctl(fd, VIDIOC_G_FMT, &fmt))
    {
        *width = fmt.fmt.pix.width;
        *height = fmt.fmt.pix.height;
        printf("getVideoFmt ");
        printf(", current format:%d", fmt.fmt.pix.pixelformat);
        printf(", width:%d", fmt.fmt.pix.width);
        printf(", height:%d", fmt.fmt.pix.height);
        printf("\n");
    }

    printf("getVideoFmt\n");
    printf(" V4L2_PIX_FMT_RGB555:%d\n", V4L2_PIX_FMT_RGB555);
    printf(" V4L2_PIX_FMT_RGB565:%d\n", V4L2_PIX_FMT_RGB565);
    printf(" V4L2_PIX_FMT_YUYV:%d\n", V4L2_PIX_FMT_YUYV);
    printf(" V4L2_PIX_FMT_UYVY:%d\n", V4L2_PIX_FMT_UYVY);
}

static int setVideoFmt(int fd, int width, int height)
{
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    int result = ioctl(fd, VIDIOC_S_FMT, &fmt);
    if (-1 == result)
    {
        printf("setVideoFmt ");
        printf(" setVideoFmt failed:%d", fmt.fmt.pix.pixelformat);
        printf(" errno:%d", errno);
        printf("\n");
    }

    return result;
}

static int initV4l2Buf(int fd, struct buffer** v4l2_buf)
{
    unsigned int i = 0;

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = V4L2_BUF_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (-1 == ioctl(fd, VIDIOC_REQBUFS, &req))
    {
        return -1;
    }

    if (req.count < V4L2_BUF_COUNT) {
        printf("initV4l2Buf Insufficient buffer memory on device\n");
        return -1;
    }

    *v4l2_buf = (struct buffer*)malloc(req.count*sizeof(struct buffer));
    if (NULL == *v4l2_buf)
    {
        return -1;
    }

    for (i = 0; i < req.count; ++i)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i; // 查询序号为mV4l2Buf的缓冲区，得到其起始物理地址和大小
        if (-1 != ioctl(fd, VIDIOC_QUERYBUF, &buf))
        {
            (*v4l2_buf)[i].length = buf.length;
            // 映射内存
            (*v4l2_buf)[i].start = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
            if (MAP_FAILED == (*v4l2_buf)[i].start)
            {
                printf("initV4l2Buf mmap failed\n");
                return -1;
            }
        }
    }

    return 0;
}

static int startCapture(int fd)
{
    unsigned int i = 0;
    for (i = 0; i < V4L2_BUF_COUNT; ++i)
    {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        // 将缓冲帧放入队列
        if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))
        {
            return -1;
        }
    }

    //开始捕捉图像数据
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))
    {
        return -1;
    }

    return 0;
}

static void stoptCapture(int fd)
{
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_STREAMOFF, &type);
}

static int readFrame(int fd, struct v4l2_buffer* buf)
{
    memset (buf, 0, sizeof(struct v4l2_buffer));
    buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf->memory = V4L2_MEMORY_MMAP;

    return ioctl(fd, VIDIOC_DQBUF, buf); // 从缓冲区取出一个缓冲帧
}

static void v4l2QBuf(int fd, struct v4l2_buffer* buf)
{
    ioctl(fd, VIDIOC_QBUF, buf); //再将其入列
}

static int open_device(int camera_index)
{
    char devName[16] = {0};
    sprintf(devName, "/dev/video%d", camera_index);
    int fd = open(devName, O_RDWR/* | O_NONBLOCK*/, 0);

    return fd;
}

static void close_device(int fd)
{
    if (fd > 0)
    {
        stoptCapture(fd);

        close(fd);
        fd = -1;
    }
}

void write2File(int channel, void* image)
{
    static int count = 0;
    IplImage* frame = (IplImage*)image;
    char outImageName[16] = {0};
    IplImage* outImage = cvCreateImage(cvGetSize(frame),frame->depth,frame->nChannels);
    // 将原图拷贝过来
    cvCopy(frame,outImage,NULL);

    //设置保存的图片名称和格式
    memset(outImageName, 0, sizeof(outImageName));
    sprintf(outImageName, "test_cam%d_%d.jpg", channel, count++);
    //保存图片
    cvSaveImage(outImageName, outImage, 0);
}

/* convert from 4:2:2 YUYV interlaced to RGB24 */
 /* based on ccvt_yuyv_bgr32() from camstream */
 #define SAT(c) \
         if (c & (~255)) { if (c < 0) c = 0; else c = 255; }

void yuyv_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst)
{
    unsigned char *s;
    unsigned char *d;
    int l, c;
    int r, g, b, cr, cg, cb, y1, y2;

    l = height;
    s = src;
    d = dst;
    while (l--) {
        c = width >> 1;
        while (c--) {
            y1 = *s++;
            cb = ((*s - 128) * 454) >> 8;
            cg = (*s++ - 128) * 88;
            y2 = *s++;
            cr = ((*s - 128) * 359) >> 8;
            cg = (cg + (*s++ - 128) * 183) >> 8;

            r = y1 + cr;
            b = y1 + cb;
            g = y1 - cg;
            SAT(r);
            SAT(g);
            SAT(b);

            *d++ = b;
            *d++ = g;
            *d++ = r;

            r = y2 + cr;
            b = y2 + cb;
            g = y2 - cg;
            SAT(r);
            SAT(g);
            SAT(b);

            *d++ = b;
            *d++ = g;
            *d++ = r;
        }
    }
}

void uyvy_to_rgb24(int width, int height, unsigned char *src, unsigned char *dst)
{
   unsigned char *s;
   unsigned char *d;
   int l, c;
   int r, g, b, cr, cg, cb, y1, y2;

   l = height;
   s = src;
   d = dst;
   while (l--) {
      c = width >> 1;
      while (c--) {
         cb = ((*s - 128) * 454) >> 8;
         cg = (*s++ - 128) * 88;
         y1 = *s++;
         cr = ((*s - 128) * 359) >> 8;
         cg = (cg + (*s++ - 128) * 183) >> 8;
         y2 = *s++;

         r = y1 + cr;
         b = y1 + cb;
         g = y1 - cg;
         SAT(r);
         SAT(g);
         SAT(b);

     *d++ = b;
     *d++ = g;
     *d++ = r;

         r = y2 + cr;
         b = y2 + cb;
         g = y2 - cg;
         SAT(r);
         SAT(g);
         SAT(b);

     *d++ = b;
     *d++ = g;
     *d++ = r;
      }
   }
}

int main(int argc, char** argv) {

    if (argc != 3) {
        printf("usage: v4l2_main camera_index frame_number.\n");
        return -1;
    }

    int camera_index = atoi(argv[1]);
    int number = atoi(argv[2]);

    int fd = open_device(camera_index);
    if (fd == -1)
    {
        printf("open device failed.\n");
        return -1;
    }

    getVideoCap(fd);
    setVideoFmt(fd, WIDTH, HEIGHT);
    getVideoFmt(fd, &WIDTH, &HEIGHT);
    if (-1 == initV4l2Buf(fd, &gV4l2Buf))
    {
        return -1;
    }

    if (-1 == startCapture(fd))
    {
        return -1;
    }

    char outImageName[16] = {0};
  
    while (1) 
    {

            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO (&fds);
            FD_SET (fd, &fds);

            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select (fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno)
                    continue;

                perror ("select");
            }

            if (0 == r) {
                fprintf (stderr, "select timeout\n");
                exit (EXIT_FAILURE);
            }

            int imageSize = WIDTH*HEIGHT*3;
            struct v4l2_buffer buf;
            if (-1 != readFrame(fd, &buf))
            {
                if (buf.index < V4L2_BUF_COUNT)
                {
                    unsigned char* buffer = (unsigned char*)(gV4l2Buf[buf.index].start);
                    unsigned char frame_buffer[imageSize];
                    yuyv_to_rgb24(WIDTH, HEIGHT, buffer, frame_buffer);
                    //memset(buffer, 0, gV4l2Buf[buf.index].length);

                    IplImage *pIplImage = cvCreateImage(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 3);
                    if (NULL != pIplImage)
                    {
                        memcpy(pIplImage->imageData, frame_buffer, imageSize);
                        write2File(camera_index, pIplImage);
                        cvReleaseImage(&pIplImage);
                    }
                }
            }
 
            v4l2QBuf(fd, &buf);

            if (--number == 0) {
                break;
            }

        usleep(10);
    }  

    return 0;  
}  
