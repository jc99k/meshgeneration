#ifndef OPENCL_UTILITIES_H
#define OPENCL_UTILITIES_H

//#define __NO_STD_VECTOR // Use cl::vector instead of STL version
#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
    #include "cl.hpp"
#else
    #include <CL/cl.hpp>
#endif


#include <string>
#include <iostream>
#include <fstream>
#include <set>

enum cl_vendor {
    VENDOR_ANY,
    VENDOR_NVIDIA,
    VENDOR_AMD,
    VENDOR_INTEL
};

class GarbageCollector {
    public:
        void addMemObject(cl::Memory * mem);
        void deleteMemObject(cl::Memory * mem);
        void deleteAllMemObjects();
        ~GarbageCollector();
    private:
        std::set<cl::Memory *> memObjects;
};


cl::Context createCLContext(cl_device_type type = CL_DEVICE_TYPE_ALL, cl_vendor vendor = VENDOR_ANY);
cl::Platform getPlatform(cl_device_type = CL_DEVICE_TYPE_ALL, cl_vendor vendor = VENDOR_ANY);
cl::Program buildProgramFromSource(cl::Context context, std::string filename, std::string buildOptions = "");
char *getCLErrorString(cl_int err);


struct OpenCLWrapper {
    cl::Context context;
    cl::CommandQueue queue;
    cl::Program program;
    cl::Device device;
    cl::Platform platform;
    GarbageCollector GC;
    
    static void information(){
        cl::Context context = createCLContext(CL_DEVICE_TYPE_GPU, VENDOR_ANY);
        auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
        std::cout << "\nUsing device: " << devices[0].getInfo<CL_DEVICE_NAME>() << std::endl;
        
        auto workItemSize = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
        auto workItemDimensions = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>();
        auto workGroupSize = devices[0].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
        auto globalMemorySize = devices[0].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
        auto maxMemoryAllocSize = devices[0].getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
        auto maxConstantBufferSize = devices[0].getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();

        printf("Max work-item size : %lu %lu %lu\n", workItemSize[0], workItemSize[1], workItemSize[2]);
        printf("Max work-group size (max items per work-group) : %lu\n", workGroupSize);
        printf("Max work-item dimensions : %u \n", workItemDimensions);
        printf("Max memory alloc size : %llu \n", maxMemoryAllocSize);
        printf("Max global memory : %llu bytes\n", globalMemorySize);
        printf("Max constant buffer size : %llu bytes\n\n", maxConstantBufferSize);

        unsigned int memorySize = (unsigned int)devices[0].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
        std::cout << "Available memory on selected device " << (double)memorySize/(1024*1024) << " MB "<< std::endl;
    }
};

#endif
