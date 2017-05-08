#include "clbase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

#define CL_ERROR !CL_SUCCESS

CLBase::CLBase()
{
    mPlatformId = NULL;
    mDeviceId = NULL;
    mContext = NULL;
    mCQ = NULL;
    mProgram = NULL;
}

CLBase::~CLBase()
{

}

int CLBase::initEnv()
{
    cl_int ret;
    ret = cl_init(&mPlatformId, &mDeviceId, &mContext, &mCQ);
    if (ret != CL_SUCCESS)
    {
        printf("\nFailed Initializing OpenCL\n");
        return -1;
    }
    else
    {
        printf("\nInitializing OpenCL Ok\n");
    }

    cl_print_info(mPlatformId, mDeviceId);

    return 0;
}

void CLBase::uninitEnv()
{
    clReleaseContext(mContext);
    clReleaseProgram(mProgram);
    clReleaseCommandQueue(mCQ);
}

int CLBase::loadKernel(char* clFileName, char* clKernelName, cl_kernel* clKernel)
{
    cl_int ret;
    struct kernel_src_str kernel_str;

    ret = cl_load_kernel_source(clFileName, &kernel_str);
    if (ret != CL_SUCCESS)
    {
        printf("\nFailed loading %s kernel\n", clFileName);
        return -1;
    }
    else
    {
        printf("\nLoading CL programs: %s Ok\n", clFileName);
    }

    ret = cl_build_program(&mProgram, &mDeviceId, mContext, &kernel_str);
    if (ret != CL_SUCCESS)
    {
         printf ("\nFailed Building kernel\n");
         return -1;
    }
    else
    {
        printf ("\nBuilding %s kernel Ok\n", clFileName);
    }

    *clKernel = clCreateKernel(mProgram, clKernelName, &ret);
    if (ret != CL_SUCCESS)
    {
         printf("\nFailed Creating program\n");
         return -1;
    }
    else
    {
        printf("\nCreating CL kernel Ok\n");
    }

    return 0;
}

void CLBase::releaseKernel(cl_kernel clKernel)
{
    clReleaseKernel(clKernel);
}

cl_int CLBase::cl_init(cl_platform_id* platform_id, cl_device_id* device_id, cl_context* context, cl_command_queue* cq)
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

cl_int CLBase::cl_print_info(cl_platform_id platform_id, cl_device_id device_id)
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

cl_int CLBase::cl_load_kernel_source(char* filename, kernel_src_str* kernel_src)
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

cl_int CLBase::cl_build_program (cl_program *program, cl_device_id *device_id, cl_context context, kernel_src_str *kernel)
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
