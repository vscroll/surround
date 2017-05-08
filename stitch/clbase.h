#ifndef CLBASE_H
#define CLBASE_H

#include <CL/cl.h>

class CLBase
{
public:
    CLBase();
    virtual ~CLBase();

protected:
    int initEnv();
    void uninitEnv();
    int loadKernel(char* clFileName, char* clKernelName, cl_kernel* clKernel);
    void releaseKernel(cl_kernel clKernel); 

private:
    // support struct to load kernel source from an external file
    struct kernel_src_str
    {
        char *src;
        size_t size;
    };

    cl_int cl_init(cl_platform_id* platform_id, cl_device_id* device_id, cl_context* context, cl_command_queue* cq);
    cl_int cl_print_info(cl_platform_id platform_id, cl_device_id device_id);
    cl_int cl_load_kernel_source(char* filename, kernel_src_str* kernel_src);
    cl_int cl_build_program(cl_program* program, cl_device_id* device_id, cl_context context, kernel_src_str* kernel);

protected:
    cl_platform_id mPlatformId;
    cl_device_id mDeviceId;
    cl_context mContext;
    cl_command_queue mCQ;
    cl_program 	mProgram;
};

#endif // CLBASE_H
