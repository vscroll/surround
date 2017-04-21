#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <CL/cl.h>
#include "stitch_cl.h"

#define CL_ERROR !CL_SUCCESS

#define TRUE 	1
#define FALSE 	0

using namespace std;

// support struct to load kernel source from an external file
struct kernel_src_str
{
    char *src;
    size_t size;
};

static cl_platform_id 	g_platform_id;
static cl_device_id 	g_device_id;
static cl_context 	g_context;
static cl_command_queue g_cq;
static cl_program 	g_program;
static cl_kernel 	g_kernel;
static cl_mem g_image_front = NULL;
static cl_mem g_image_rear = NULL;
static cl_mem g_image_left = NULL;
static cl_mem g_image_right = NULL;
static cl_mem g_image_mask = NULL;
static cl_mem g_image_map_x = NULL;
static cl_mem g_image_map_y = NULL;
static cl_mem g_image_pano2d = NULL;

// one dimensional work-items
static int g_dimension = 2;

// our problem size
static size_t g_global = 16;

// preferred work-group size
static size_t g_local = 16;

static cl_int cl_build_program (cl_program *program, cl_device_id *device_id, cl_context context, kernel_src_str *kernel);
static cl_int cl_print_info (cl_platform_id platform_id, cl_device_id device_id);
static cl_int cl_init (cl_platform_id *platform_id, cl_device_id *device_id, cl_context *context, cl_command_queue *cq);
static cl_int cl_load_kernel_source (char *filename, kernel_src_str *kernel_src);

#if CL_HELLOWORLD
static cl_mem g_helloworld_in = NULL;
static cl_mem g_helloworld_out = NULL;

int stitch_cl_new_helloworld_buffer(int width, int height)
{
    cl_int ret;
    printf ("\nAllocating buffers...");
    g_helloworld_in = clCreateBuffer (g_context, CL_MEM_READ_ONLY, width*height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("Failed Allocation  g_helloworld_in buffer\n");
        return -1;
    }

    g_helloworld_out = clCreateBuffer (g_context, CL_MEM_WRITE_ONLY, width*height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Allocation hello_world output buffer\n");
         return -1;
    }
    else
    {
        printf (" Ok\n");
    }

    clSetKernelArg (g_kernel, 0, sizeof(cl_mem), &g_helloworld_in);
    clSetKernelArg (g_kernel, 1, sizeof(cl_mem), &g_helloworld_out);
    clSetKernelArg (g_kernel, 2, sizeof(cl_int), &width);
    clSetKernelArg (g_kernel, 3, sizeof(cl_int), &height);
}

int stitch_cl_delete_helloworld_buffer()
{
    clReleaseMemObject (g_helloworld_in);
    clReleaseMemObject (g_helloworld_out);
}

int stitch_cl_helloworld(int helloworld_in[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX],
                         int helloworld_out[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX],
                         int width,
                         int height)
{
    cl_int ret;
    ret = clEnqueueWriteBuffer(g_cq, g_helloworld_in, CL_TRUE, 0, sizeof(width*height*sizeof(int)), (void*)helloworld_in, 0, NULL, NULL);
    if (ret != CL_SUCCESS)
    {
        printf ("\nError writing input buffer\n");
    }

   ret = clEnqueueNDRangeKernel (g_cq, g_kernel, g_dimension, NULL, &g_global, &g_local, 0, NULL, NULL);
   if  (ret == CL_SUCCESS)
   {
       ret = clEnqueueReadBuffer(g_cq, g_image_pano2d, CL_TRUE, 0, sizeof(width*height*sizeof(int)), (void*)g_helloworld_out, 0, NULL, NULL);
       printf ("\nOk reading output buffer\n");
   }
   else
   {
       printf ("\nError reading output buffer\n");
   }

   clFlush(g_cq);
   clFinish(g_cq);
}

#endif

int stitch_cl_init(char* cl_file_name, char* cl_kernel_name)
{
    struct kernel_src_str kernel_str;

    printf ("\nInitializing OpenCL:");
    cl_int ret;
    ret = cl_init (&g_platform_id, &g_device_id, &g_context, &g_cq);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed Initializing OpenCL\n");
        return -1;
    }
    else
    {
        printf (" Ok\n");
    }

    ret = cl_print_info (g_platform_id, g_device_id);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed reading OpenCL Info\n");
    }

    printf ("\nLoading CL programs: %s", cl_file_name);
    ret = cl_load_kernel_source (cl_file_name, &kernel_str);
    if (ret != CL_SUCCESS)
    {
        printf ("\nFailed loading %s kernel\n", cl_file_name);
        return -1;
    }
    else
    {
        printf (" Ok\n");
    }

    printf ("\nBuilding %s kernel: ", cl_file_name);
    ret = cl_build_program (&g_program, &g_device_id, g_context, &kernel_str);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Building hello_world kernel\n");
         return -1;
    }
    else
    {
        printf (" Ok\n");
    }

    printf ("\nCreating CL kernel...");
    g_kernel = clCreateKernel (g_program, cl_kernel_name, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Creating hellow_world program\n");
         return -1;
    }
    else
    {
        printf (" Ok\n");
    }

    return 0;
}

int stitch_cl_new_pano2d_buffer(int in_side_width, int in_side_height,
                                int out_side_width, int out_side_height,
                                int out_pano2d_width, int  out_pano2d_height)
{
    cl_int ret;
    printf ("\nAllocating buffers...");
    g_image_front = clCreateBuffer (g_context, CL_MEM_READ_ONLY, in_side_width*in_side_height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
        printf ("Failed Allocation  image_front buffer\n");
        return -1;
    }

    g_image_rear = clCreateBuffer (g_context, CL_MEM_READ_ONLY, in_side_width*in_side_height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Allocation  image_rear buffer\n");
         return -1;
    }

    g_image_left = clCreateBuffer (g_context, CL_MEM_READ_ONLY, in_side_width*in_side_height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Allocation  image_left buffer\n");
         return -1;
    }

    g_image_right = clCreateBuffer (g_context, CL_MEM_READ_ONLY, in_side_width*in_side_height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Allocation  image_right buffer\n");
         return -1;
    }

    g_image_mask  = clCreateBuffer (g_context, CL_MEM_READ_ONLY, out_pano2d_width*out_pano2d_height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Allocation  image_right buffer\n");
         return -1;
    }

    g_image_map_x = clCreateBuffer (g_context, CL_MEM_READ_ONLY, out_pano2d_width*out_pano2d_height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Allocation  image_right buffer\n");
         return -1;
    }

    g_image_map_y = clCreateBuffer (g_context, CL_MEM_READ_ONLY, out_pano2d_width*out_pano2d_height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Allocation  image_right buffer\n");
         return -1;
    }

    g_image_pano2d = clCreateBuffer (g_context, CL_MEM_WRITE_ONLY, out_pano2d_width*out_pano2d_height*sizeof(int), NULL, &ret);
    if (ret != CL_SUCCESS)
    {
         printf ("Failed Allocation hello_world output buffer\n");
         return -1;
    }
    else
    {
        printf (" Ok\n");
    }

    clSetKernelArg (g_kernel, 0, sizeof(cl_mem), &g_image_front);
    clSetKernelArg (g_kernel, 1, sizeof(cl_mem), &g_image_rear);
    clSetKernelArg (g_kernel, 2, sizeof(cl_mem), &g_image_left);
    clSetKernelArg (g_kernel, 3, sizeof(cl_mem), &g_image_right);
    clSetKernelArg (g_kernel, 4, sizeof(cl_mem), &g_image_mask);
    clSetKernelArg (g_kernel, 5, sizeof(cl_mem), &g_image_map_x);
    clSetKernelArg (g_kernel, 6, sizeof(cl_mem), &g_image_map_y);
    clSetKernelArg (g_kernel, 7, sizeof(cl_mem), &out_pano2d_width);
    clSetKernelArg (g_kernel, 8, sizeof(cl_mem), &out_pano2d_height);
    clSetKernelArg (g_kernel, 9, sizeof(cl_mem), &g_image_pano2d);
}

void stitch_cl_delete_pano2d_buffer()
{
    clReleaseMemObject (g_image_front);
    clReleaseMemObject (g_image_rear);
    clReleaseMemObject (g_image_left);
    clReleaseMemObject (g_image_right);
    clReleaseMemObject (g_image_mask);
    clReleaseMemObject (g_image_map_x);
    clReleaseMemObject (g_image_map_y);
    clReleaseMemObject (g_image_pano2d);

    printf ("\nfree buffers ok");
}

int g_image_front_array[VIDEO_SIDE_RES_Y_MAX][VIDEO_SIDE_RES_X_MAX];
int g_image_rear_array[VIDEO_SIDE_RES_Y_MAX][VIDEO_SIDE_RES_X_MAX];
int g_image_left_array[VIDEO_SIDE_RES_Y_MAX][VIDEO_SIDE_RES_X_MAX];
int g_image_right_array[VIDEO_SIDE_RES_Y_MAX][VIDEO_SIDE_RES_X_MAX];
int g_image_mapx_array[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX];
int g_image_mapy_array[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX];
int g_image_mask_array[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX];

int stitch_cl_2d(const std::vector<cv::Mat>& side_imgs,
                 const cv::Mat& map_x, const cv::Mat& map_y,
                 const cv::Mat& mask,
                 int in_side_width, int in_side_height,
                 int out_pano2d_width, int out_pano2d_height,
                 int image_pano2d[VIDEO_PANO2D_RES_Y_MAX][VIDEO_PANO2D_RES_X_MAX]
                 )
{
    for (int i = 0; i < side_imgs[0].rows; ++i)
    {
        for (int j = 0; j < side_imgs[0].cols; ++j)
        {
            g_image_front_array[i][j] = side_imgs[0].ptr<int>(i)[j];
        }
    }

    for (int i = 0; i < side_imgs[1].rows; ++i)
    {
        for (int j = 0; j < side_imgs[1].cols; ++j)
        {
            g_image_rear_array[i][j] = side_imgs[1].ptr<int>(i)[j];
        }
    }

    for (int i = 0; i < side_imgs[2].rows; ++i)
    {
        for (int j = 0; j < side_imgs[2].cols; ++j)
        {
            g_image_left_array[i][j] = side_imgs[2].ptr<int>(i)[j];
        }
    }

    for (int i = 0; i < side_imgs[3].rows; ++i)
    {
        for (int j = 0; j < side_imgs[3].cols; ++j)
        {
            g_image_right_array[i][j] = side_imgs[3].ptr<int>(i)[j];
        }
    }

    for (int i = 0; i < out_pano2d_height; ++i)
    {
        for (int j = 0; j < out_pano2d_width;  ++j)
        {
            int flag = mask.ptr<uchar>(i)[j];
            g_image_mask_array[i][j] = flag;
        }
    }

    for (int i = 0; i < out_pano2d_height; ++i)
    {
        for (int j = 0; j < out_pano2d_width; ++j)
        {
            g_image_mapx_array[i][j] = map_x.ptr<int>(i)[j];
        }
    }

    for (int i = 0; i < out_pano2d_height; ++i)
    {
        for (int j = 0; j < out_pano2d_width; ++j)
        {
            g_image_mapy_array[i][j] = map_y.ptr<int>(i)[j];
        }
    }

     cl_int ret;
     ret = clEnqueueWriteBuffer(g_cq, g_image_front, CL_TRUE, 0, sizeof(in_side_width*in_side_height*sizeof(int)), (void*)g_image_front_array, 0, NULL, NULL);
     if (ret != CL_SUCCESS)
     {
         printf ("\nError writing input buffer\n");
     }

     ret = clEnqueueWriteBuffer(g_cq, g_image_rear, CL_TRUE, 0, sizeof(in_side_width*in_side_height*sizeof(int)), (void*)g_image_rear_array, 0, NULL, NULL);
     if (ret != CL_SUCCESS)
     {
         printf ("\nError writing input buffer\n");
     }

     ret = clEnqueueWriteBuffer(g_cq, g_image_left, CL_TRUE, 0, sizeof(in_side_width*in_side_height*sizeof(int)), (void*)g_image_left_array, 0, NULL, NULL);
     if (ret != CL_SUCCESS)
     {
         printf ("\nError writing input buffer\n");
     }

     ret = clEnqueueWriteBuffer(g_cq, g_image_right, CL_TRUE, 0, sizeof(in_side_width*in_side_height*sizeof(int)), (void*)g_image_right_array, 0, NULL, NULL);
     if (ret != CL_SUCCESS)
     {
         printf ("\nError writing input buffer\n");
     }

     ret = clEnqueueWriteBuffer(g_cq, g_image_mask, CL_TRUE, 0, sizeof(out_pano2d_width*out_pano2d_height*sizeof(int)), (void*)g_image_mask_array, 0, NULL, NULL);
     if (ret != CL_SUCCESS)
     {
         printf ("\nError writing input buffer\n");
     }

     ret = clEnqueueWriteBuffer(g_cq, g_image_map_x, CL_TRUE, 0, sizeof(out_pano2d_width*out_pano2d_height*sizeof(int)), (void*)g_image_mapx_array, 0, NULL, NULL);
     if (ret != CL_SUCCESS)
     {
         printf ("\nError writing input buffer\n");
     }

     ret = clEnqueueWriteBuffer(g_cq, g_image_map_y, CL_TRUE, 0, sizeof(out_pano2d_width*out_pano2d_height*sizeof(int)), (void*)g_image_mapy_array, 0, NULL, NULL);
     if (ret != CL_SUCCESS)
     {
         printf ("\nError writing input buffer\n");
     }

    ret = clEnqueueNDRangeKernel (g_cq, g_kernel, g_dimension, NULL, &g_global, &g_local, 0, NULL, NULL);
    if  (ret == CL_SUCCESS)
    {
        ret = clEnqueueReadBuffer(g_cq, g_image_pano2d, CL_TRUE, 0, sizeof(out_pano2d_width*out_pano2d_height*sizeof(int)), (void*)image_pano2d, 0, NULL, NULL);
        printf ("\nOk reading output buffer\n");
    }
    else
    {
        printf ("\nError reading output buffer\n");
    }

    clFlush(g_cq);
    clFinish(g_cq);

    return 0;
}

void stitch_cl_uninit()
{
    clReleaseContext(g_context);
    clReleaseProgram(g_program);
    clReleaseCommandQueue(g_cq);
    clReleaseKernel (g_kernel);

    stitch_cl_delete_pano2d_buffer();
}

cl_int cl_init (cl_platform_id *platform_id, cl_device_id *device_id, cl_context *context, cl_command_queue *cq)
{
    cl_uint  platforms, devices;
    cl_int error;

    //-------------------------------------------
    // cl_int clGetPlatformIDs (cl_uint num_entries, cl_platform_id *platforms, cl_uint *num_platforms)
    //
    // num_entries
    //	The number of cl_platform_id entries that can be added to platforms. If platforms is not NULL, the num_entries must be greater than zero.
    //
    // platforms
    //	Returns a list of OpenCL platforms found. The cl_platform_id values returned in platforms can be used to identify a specific OpenCL platform. If platforms argument is NULL, this argument is ignored. The number of OpenCL platforms returned is the mininum of the value specified by num_entries or the number of OpenCL platforms available.
    //
    // num_platforms
    //	Returns the number of OpenCL platforms available. If num_platforms is NULL, this argument is ignored.
    //--------------------------------------------
    error = clGetPlatformIDs (1, platform_id, &platforms);
    if (error != CL_SUCCESS)
        return CL_ERROR;

    //--------------------------------------------
    // cl_int clGetDeviceIDs (cl_platform_id platform, cl_device_type device_type, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices)
    //
    // platform
    //	Refers to the platform ID returned by clGetPlatformIDs or can be NULL. If platform is NULL, the behavior is implementation-defined.
    //
    // device_type
    //	A bitfield that identifies the type of OpenCL device. The device_type can be used to query specific OpenCL devices or all OpenCL devices available. The valid values for device_type are specified as:
    //	CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_GPU, GL_DEVICE_TYPE_ACCELERATOR, CL_DEVICE_TYPE_DEFAULT, GL_DEVICE_TYPE_ALL
    //
    // num_entries
    //	The number of cl_device entries that can be added to devices. If devices is not NULL, the num_entries must be greater than zero.
    //
    // devices
    //	A list of OpenCL devices found. The cl_device_id values returned in devices can be used to identify a specific OpenCL device. If devices argument is NULL, this argument is ignored. The number of OpenCL devices returned is the mininum of the value specified by num_entries or the number of OpenCL devices whose type matches device_type.
    //
    // num_devices
    //	The number of OpenCL devices available that match device_type. If num_devices is NULL, this argument is ignored.
    //--------------------------------------------
    error = clGetDeviceIDs ((* platform_id), CL_DEVICE_TYPE_GPU, 1, device_id, &devices);
    if (error != CL_SUCCESS)
        return CL_ERROR;

    //--------------------------------------------
    // cl_context clCreateContext (cl_context_properties *properties, cl_uint num_devices, const cl_device_id *devices, void *pfn_notify (const char *errinfo, const void *private_info, size_t cb, void *user_data), 	void *user_data, cl_int *errcode_ret)
    //
    //properties
    //	Specifies a list of context property names and their corresponding values. Each property name is immediately followed by the corresponding desired value. The list is terminated with 0. properties can be NULL in which case the platform that is selected is implementation-defined.
    //
    // num_devices
    //	The number of devices specified in the devices argument.
    //
    // devices
    //	A pointer to a list of unique devices returned by clGetDeviceIDs for a platform.
    //
    // pfn_notify
    //	A callback function that can be registered by the application. This callback function will be used by the OpenCL implementation to report information on errors that occur in this context. This callback function may be called asynchronously by the OpenCL implementation. It is the application's responsibility to ensure that the callback function is thread-safe. If pfn_notify is NULL, no callback function is registered. The parameters to this callback function are:
    //	errinfo is a pointer to an error string.
    //	private_info and cb represent a pointer to binary data that is returned by the OpenCL implementation that can be used to log additional information helpful in debugging the error.
    //	user_data is a pointer to user supplied data.
    //
    // user_data
    //	Passed as the user_data argument when pfn_notify is called. user_data can be NULL.
    //
    // errcode_ret
    //	Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
    //----------------------------------------------
    cl_context_properties properties[] = {CL_CONTEXT_PLATFORM, (cl_context_properties)(* platform_id), 0};
    (* context) = clCreateContext (properties, 1, device_id, NULL, NULL, &error);

    if (error != CL_SUCCESS)
        return CL_ERROR;

    //----------------------------------------------
    // cl_command_queue clCreateCommandQueue (cl_context context, cl_device_id device, cl_command_queue_properties properties, cl_int *errcode_ret)
    //
    // context
    //	Must be a valid OpenCL context.
    //
    // device
    //	Must be a device associated with context. It can either be in the list of devices specified when context is created using clCreateContext or have the same device type as the device type specified when the context is created using clCreateContextFromType.
    //
    // properties
    //	Specifies a list of properties for the command-queue. This is a bit-field. Only command-queue properties specified below can be set in properties; otherwise the value specified in properties is considered to be not valid.
    //	CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, CL_QUEUE_PROFILING_ENABLE
    //-----------------------------------------------
    (* cq) = clCreateCommandQueue ((* context), (* device_id), 0, &error);
    if (error != CL_SUCCESS)
        return CL_ERROR;

    return CL_SUCCESS;
}

cl_int cl_print_info (cl_platform_id platform_id, cl_device_id device_id)
{
    uint i;
    char buffer[10240];
    cl_int ret;

    //---------------------------------------
    //cl_int clGetPlatformInfo (cl_platform_id platform, cl_platform_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
    //
    // platform
    // 	The platform ID returned by clGetPlatformIDs or can be NULL. If platform is NULL, the behavior is implementation-defined.
    //
    // param_name
    //	An enumeration constant that identifies the platform information being queried. See Khronos documentation for more information.
    //
    // param_value_size
    //	Specifies the size in bytes of memory pointed to by param_value. This size in bytes must be greater than or equal to size of return type specified in the table below.
    //
    // param_value
    //	A pointer to memory location where appropriate values for a given param_value will be returned. Acceptable param_value values are listed in the table below. If param_value is NULL, it is ignored.
    //
    // param_value_size_ret
    //	Returns the actual size in bytes of data being queried by param_value. If param_value_size_ret is NULL, it is ignored
    //-----------------------------------------

    printf ("\n-=-=-=- Platform Information -=-=-=-\n\n");
    ret = clGetPlatformInfo (platform_id, CL_PLATFORM_NAME, 10240, buffer, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Platform Name: %s\n", buffer);

    ret = clGetPlatformInfo (platform_id, CL_PLATFORM_PROFILE, 10240, buffer, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Platform Profile: %s\n", buffer);

    ret = clGetPlatformInfo (platform_id, CL_PLATFORM_VERSION, 10240, buffer, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Platform Version: %s\n", buffer);

    ret = clGetPlatformInfo (platform_id, CL_PLATFORM_VENDOR, 10240, buffer, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Platform Vendor: %s\n", buffer);

    printf ("\n-=-=-=- Device Information -=-=-=-\n\n");
    ret = clGetDeviceInfo( device_id, CL_DEVICE_NAME, 10240, buffer, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Device Name: %s\n", buffer);

    //-------------------------------------------
    // cl_int clGetDeviceInfo (cl_device_id device, cl_device_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
    //
    // device
    //	Refers to the device returned by clGetDeviceIDs.
    //
    // param_name
    //	An enumeration constant that identifies the device information being queried. See Khronos documentation for more information
    //
    // param_value
    //	A pointer to memory location where appropriate values for a given param_name as specified in the table below will be returned. If param_value is NULL, it is ignored.
    //
    // param_value_size
    //	Specifies the size in bytes of memory pointed to by param_value. This size in bytes must be greater than or equal to size of return type specified in the table below.
    //
    // param_value_size_ret
    //	Returns the actual size in bytes of data being queried by param_value. If param_value_size_ret is NULL, it is ignored
    //--------------------------------------------
    ret = clGetDeviceInfo (device_id, CL_DEVICE_PROFILE, 10240, buffer, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;

    printf ("Device Profile: %s\n", buffer);

    ret = clGetDeviceInfo (device_id, CL_DEVICE_VERSION, 10240, buffer, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Device Version: %s\n", buffer);

    ret = clGetDeviceInfo (device_id, CL_DEVICE_VENDOR, 10240, buffer, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Device Vendor: %s\n", buffer);

    ret = clGetDeviceInfo (device_id, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof (uint), &i, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Device Max Work Item Dimensions: %u-D\n", i);

    ret = clGetDeviceInfo (device_id, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof (uint), &i, NULL);
    if (ret != CL_SUCCESS)
        return CL_ERROR;
    printf ("Device Max Work Group Size: %u\n", i);

//	ret = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof (uint), &i, NULL);
//	if (ret != CL_SUCCESS)
//		return 0;
//	printf ("Device Max Work Item Sizes: %u\n", i);

//	ret = clGetDeviceInfo(device_id, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof (uint), &i, NULL);
//	if (ret != CL_SUCCESS)
//		return 0;
//	printf ("Device Global Memory Size: %u\n", i);
    printf ("\n-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");

    return CL_SUCCESS;
}

cl_int cl_load_kernel_source (char *filename, kernel_src_str *kernel_src)
{

    FILE *fp = NULL;

    fp = fopen (filename, "rb");

    if (fp == NULL)
    {
        printf ("\nFailed to open: %s\n", filename);
        return CL_ERROR;
    }

    fseek (fp, 0, SEEK_END);
    kernel_src->size = ftell (fp);
    rewind (fp);

    // prevent re-allocation
    //if (kernel->src) free (kernel->src);
    kernel_src->src =  (char *) malloc (sizeof (char) * kernel_src->size+1);
    if (! kernel_src->src)
    {
        printf ("\nError Allocating memory to load CL program");
        return CL_ERROR;
    }
    fread (kernel_src->src, 1, kernel_src->size, fp);

    kernel_src->src[kernel_src->size] = '\0';
    fclose (fp);

    return CL_SUCCESS;
}

cl_int cl_build_program (cl_program *program, cl_device_id *device_id, cl_context context, kernel_src_str *kernel)
{
    cl_int error = CL_SUCCESS;

    //----------------------------------------------
    // cl_program clCreateProgramWithSource (cl_context context, cl_uint count, const char **strings, const size_t *lengths, cl_int *errcode_ret)
    //
    // Parameters
    //	context
    //		Must be a valid OpenCL context.
    //
    //	strings
    //		An array of count pointers to optionally null-terminated character strings that make up the source code.
    //
    //	lengths
    //		An array with the number of chars in each string (the string length). If an element in lengths is zero, its accompanying string is null-terminated. If lengths is NULL, all strings in the strings argument are considered null-terminated. Any length value passed in that is greater than zero excludes the null terminator in its count.
    //
    //	errcode_ret
    //		Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
    //------------------------------------------------
    (* program) = clCreateProgramWithSource (context, 1, (const char **)&kernel->src, &kernel->size, &error);
    if (error != CL_SUCCESS)
    {
        return CL_ERROR;
    }

    //------------------------------------------------
    // cl_int clBuildProgram (cl_program program, cl_uint num_devices, const cl_device_id *device_list, const char *options, void (*pfn_notify)(cl_program, void *user_data), void *user_data)
    //
    // Parameters
    //	program
    //		The program object
    //
    //	device_list
    //		A pointer to a list of devices that are in program. If device_list is NULL value, the program executable is built for all devices associated with program for which a source or binary has been loaded. If device_list is a non-NULL value, the program executable is built for devices specified in this list for which a source or binary has been loaded.
    //
    //	num_devices
    //		The number of devices listed in device_list.
    //
    //	options
    //		A pointer to a string that describes the build options to be used for building the program executable. The list of supported options is described in "Build Options" below.
    //
    //	pfn_notify
    //		A function pointer to a notification routine. The notification routine is a callback function that an application can register and which will be called when the program executable has been built (successfully or unsuccessfully). If pfn_notify is not NULL, clBuildProgram does not need to wait for the build to complete and can return immediately. If pfn_notify is NULL, clBuildProgram does not return until the build has completed. This callback function may be called asynchronously by the OpenCL implementation. It is the application's responsibility to ensure that the callback function is thread-safe.
    //
    //	user_data
    //		Passed as an argument when pfn_notify is called. user_data can be NULL.
    //-------------------------------------------------
    error = clBuildProgram ((* program), 1, device_id, "", NULL, NULL);
    if (error < 0)
    {
        //---------------------------------------------------
        // cl_int clGetProgramBuildInfo ( cl_program  program, cl_device_id  device, cl_program_build_info  param_name, size_t  param_value_size, void  *param_value, size_t  *param_value_size_ret)
        // program
        //	Specifies the program object being queried.
        //
        // device
        //	Specifies the device for which build information is being queried. device must be a valid device associated with program.
        //
        // param_name
        //	Specifies the information to query. The list of supported param_name is: CL_PROGRAM_BUILD_STATUS, CL_PROGRAM_BUILD_OPTIONS, CL_PROGRAM_BUILD_LOG
        //
        // param_value
        //	A pointer to memory where the appropriate result being queried is returned. If param_value is NULL, it is ignored.
        //
        // param_value_size
        //	Specifies the size in bytes of memory pointed to by param_value. This size must be greater than or equal to the size of return type as described in the table above.
        //
        // param_value_size_ret
        //	Returns the actual size in bytes of data copied to param_value. If param_value_size_ret is NULL, it is ignored.
        //---------------------------------------------------
        clGetProgramBuildInfo((* program), (* device_id), CL_PROGRAM_BUILD_LOG, kernel->size, kernel->src, NULL);
        printf ("\n%s", kernel->src);
    }

    return error;
}
