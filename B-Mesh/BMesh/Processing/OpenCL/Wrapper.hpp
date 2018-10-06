//
//#define __CL_ENABLE_EXCEPTIONS
//
//#include "cl.hpp"
//
//#ifdef __APPLE__
//    #include <OpenCL/opencl.h>
//#endif
//
//#include <string>
//#include <fstream>
//#include <iterator>
//#include <vector>
//#include <assert.h>


//class OpenCLWrapper{
//private:
//    std::string readKernel(std::string kernelFile){
//        std::ifstream ifs(kernelFile);
//        std::string source((std::istreambuf_iterator<char>(ifs) ),
//                           (std::istreambuf_iterator<char>()) );
//        return source;
//    }
//    
//    cl::Program program;
//    cl::Context context;
//    cl::Kernel kernel;
//    std::vector<cl::Device> devices;
//    
//public:
//    char *getCLErrorString(cl_int err){
//        switch (err) {
//            case CL_SUCCESS:                          return (char *) "Success!";
//            case CL_DEVICE_NOT_FOUND:                 return (char *) "Device not found.";
//            case CL_DEVICE_NOT_AVAILABLE:             return (char *) "Device not available";
//            case CL_COMPILER_NOT_AVAILABLE:           return (char *) "Compiler not available";
//            case CL_MEM_OBJECT_ALLOCATION_FAILURE:    return (char *) "Memory object allocation failure";
//            case CL_OUT_OF_RESOURCES:                 return (char *) "Out of resources";
//            case CL_OUT_OF_HOST_MEMORY:               return (char *) "Out of host memory";
//            case CL_PROFILING_INFO_NOT_AVAILABLE:     return (char *) "Profiling information not available";
//            case CL_MEM_COPY_OVERLAP:                 return (char *) "Memory copy overlap";
//            case CL_IMAGE_FORMAT_MISMATCH:            return (char *) "Image format mismatch";
//            case CL_IMAGE_FORMAT_NOT_SUPPORTED:       return (char *) "Image format not supported";
//            case CL_BUILD_PROGRAM_FAILURE:            return (char *) "Program build failure";
//            case CL_MAP_FAILURE:                      return (char *) "Map failure";
//            case CL_INVALID_VALUE:                    return (char *) "Invalid value";
//            case CL_INVALID_DEVICE_TYPE:              return (char *) "Invalid device type";
//            case CL_INVALID_PLATFORM:                 return (char *) "Invalid platform";
//            case CL_INVALID_DEVICE:                   return (char *) "Invalid device";
//            case CL_INVALID_CONTEXT:                  return (char *) "Invalid context";
//            case CL_INVALID_QUEUE_PROPERTIES:         return (char *) "Invalid queue properties";
//            case CL_INVALID_COMMAND_QUEUE:            return (char *) "Invalid command queue";
//            case CL_INVALID_HOST_PTR:                 return (char *) "Invalid host pointer";
//            case CL_INVALID_MEM_OBJECT:               return (char *) "Invalid memory object";
//            case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:  return (char *) "Invalid image format descriptor";
//            case CL_INVALID_IMAGE_SIZE:               return (char *) "Invalid image size";
//            case CL_INVALID_SAMPLER:                  return (char *) "Invalid sampler";
//            case CL_INVALID_BINARY:                   return (char *) "Invalid binary";
//            case CL_INVALID_BUILD_OPTIONS:            return (char *) "Invalid build options";
//            case CL_INVALID_PROGRAM:                  return (char *) "Invalid program";
//            case CL_INVALID_PROGRAM_EXECUTABLE:       return (char *) "Invalid program executable";
//            case CL_INVALID_KERNEL_NAME:              return (char *) "Invalid kernel name";
//            case CL_INVALID_KERNEL_DEFINITION:        return (char *) "Invalid kernel definition";
//            case CL_INVALID_KERNEL:                   return (char *) "Invalid kernel";
//            case CL_INVALID_ARG_INDEX:                return (char *) "Invalid argument index";
//            case CL_INVALID_ARG_VALUE:                return (char *) "Invalid argument value";
//            case CL_INVALID_ARG_SIZE:                 return (char *) "Invalid argument size";
//            case CL_INVALID_KERNEL_ARGS:              return (char *) "Invalid kernel arguments";
//            case CL_INVALID_WORK_DIMENSION:           return (char *) "Invalid work dimension";
//            case CL_INVALID_WORK_GROUP_SIZE:          return (char *) "Invalid work group size";
//            case CL_INVALID_WORK_ITEM_SIZE:           return (char *) "Invalid work item size";
//            case CL_INVALID_GLOBAL_OFFSET:            return (char *) "Invalid global offset";
//            case CL_INVALID_EVENT_WAIT_LIST:          return (char *) "Invalid event wait list";
//            case CL_INVALID_EVENT:                    return (char *) "Invalid event";
//            case CL_INVALID_OPERATION:                return (char *) "Invalid operation";
//            case CL_INVALID_GL_OBJECT:                return (char *) "Invalid OpenGL object";
//            case CL_INVALID_BUFFER_SIZE:              return (char *) "Invalid buffer size";
//            case CL_INVALID_MIP_LEVEL:                return (char *) "Invalid mip-map level";
//            default:                                  return (char *) "Unknown";
//        }
//    }
//    
//    static void information() {
//        try {
//            std::vector<cl::Platform> platforms;
//            cl::Platform::get(&platforms);
//            
//            if(platforms.size() == 0){ exit(EXIT_FAILURE);}
//            
//            cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
//            auto context = cl::Context(CL_DEVICE_TYPE_GPU, properties);
//            auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
//            
//            auto workItemSize = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
//            auto workItemDimensions = devices[0].getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>();
//            auto workGroupSize = devices[0].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
//            auto globalMemorySize = devices[0].getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
//            auto maxMemoryAllocSize = devices[0].getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
//            auto maxConstantBufferSize = devices[0].getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
//            
//            printf("\n\tMax work-item size : %lu %lu %lu\n", workItemSize[0], workItemSize[1], workItemSize[2]);
//            printf("\tMax work-group size (max items per work-group) : %lu\n", workGroupSize);
//            printf("\tMax work-item dimensions : %u \n", workItemDimensions);
//            printf("\tMax memory alloc size : %llu \n", maxMemoryAllocSize);
//            printf("\tMax global memory : %llu bytes\n", globalMemorySize);
//            printf("\tMax constant buffer size : %llu bytes\n\n", maxConstantBufferSize);
//        } catch (cl::Error error) {
//            std::cerr << "ERROR :" << error.what() << " (" << error.err() << ")\n";
//            exit(EXIT_FAILURE);
//        }
//    }
//    
//    OpenCLWrapper(std::string kernelFile, std::string kernelFunction){
//        
//        cl_int status = CL_SUCCESS;
//        try {
//            std::vector<cl::Platform> platforms;
//            cl::Platform::get(&platforms);
//            
//            if(platforms.size() == 0){ exit(EXIT_FAILURE);}
//            
//            cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(), 0};
//            context = cl::Context(CL_DEVICE_TYPE_GPU, properties);
//            
//            devices = context.getInfo<CL_CONTEXT_DEVICES>();
//            
//            std::string kernelSource    = readKernel(kernelFile);
//            size_t kernelSourceSize     = kernelSource.size();
//            
//            assert(kernelSource.size() > 0);
//            
//            cl::Program::Sources source(1, std::make_pair(kernelSource.c_str(), kernelSourceSize));
//            program = cl::Program(context, source);
////            program.build(devices, "-I ....");
//            program.build(devices);
//            
//            kernel = cl::Kernel(program, kernelFunction.c_str(), &status);
//            
//        } catch (cl::Error error) {
//            std::cerr << "ERROR :" << error.what() << " (" << error.err() << ")\n";
//            std::cerr << getCLErrorString(error.err()) << std::endl;
//
//            exit(EXIT_FAILURE);
//        }
//    }
//    
//    
//    void edges(brahand::ImageSize size, brahand::ArrayPointer<brahand::PixelType> input, brahand::ArrayPointer<brahand::PixelType> isovalues, brahand::ArrayPointer<brahand::PixelType> &output ){
//        cl_int status = CL_SUCCESS;
//        
//        try {
//            cl::Buffer inputBuffer( context, CL_MEM_READ_ONLY, sizeof(brahand::PixelType) * input.count );
//            cl::Buffer isovaluesBuffer( context, CL_MEM_READ_ONLY, sizeof(brahand::PixelType) * isovalues.count );
//            cl::Buffer outputBuffer( context,CL_MEM_WRITE_ONLY, sizeof(brahand::PixelType) * input.count );
//
//            kernel.setArg(0, inputBuffer);
//            kernel.setArg(1, isovaluesBuffer);
//            kernel.setArg(2, sizeof(brahand::uint), &isovalues.count);
//            kernel.setArg(3, outputBuffer);
//
//            cl::Event event;
//            cl::CommandQueue queue(context, devices[0], 0, &status);
//            queue.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, sizeof(brahand::PixelType) * input.count, input.content.get());
//            queue.enqueueWriteBuffer(isovaluesBuffer, CL_TRUE, 0, sizeof(brahand::PixelType) * isovalues.count, isovalues.content.get());
//
//            queue.enqueueNDRangeKernel(kernel,
//                                       cl::NullRange,
//                                       cl::NDRange(size.width, size.height, size.depth),
//                                       cl::NDRange(1,1,1),
//                                       NULL,
//                                       &event);
//
//            event.wait();
//            queue.finish();
//            queue.enqueueReadBuffer(outputBuffer,CL_TRUE, 0, sizeof(brahand::PixelType) * input.count, output.content.get() );
//            
//        } catch (cl::Error error) {
//            std::cerr << "ERROR :" << error.what() << " (" << error.err() << ")\n";
//            std::cerr << getCLErrorString(error.err()) << std::endl;
//            exit(EXIT_FAILURE);
//        }
//    }
//    
//    void naiveDistanceTransform(brahand::ImageSize size, brahand::IndicesArray edges, brahand::ArrayPointer<float> &output ){
//        cl_int status = CL_SUCCESS;
//        
//        try {
//            cl::Buffer inputBuffer( context, CL_MEM_READ_ONLY, sizeof(brahand::uint) * edges.count );
//            cl::Buffer outputBuffer( context,CL_MEM_WRITE_ONLY, sizeof(float) * output.count );
//
//            kernel.setArg(0, inputBuffer);
//            kernel.setArg(1, sizeof(brahand::uint), &edges.count);
//            kernel.setArg(2, outputBuffer);
//
//            cl::Event event;
//            cl::CommandQueue queue(context, devices[0], 0, &status);
//            queue.enqueueWriteBuffer(inputBuffer, CL_TRUE, 0, sizeof(brahand::uint) * edges.count, edges.content.get());
//
//            queue.enqueueNDRangeKernel(kernel,
//                                       cl::NullRange,
//                                       cl::NDRange(size.width, size.height, size.depth),
//                                       cl::NDRange(1,1,1),
//                                       NULL,
//                                       &event);
//            
//            event.wait();
//            queue.finish();
//            queue.enqueueReadBuffer(outputBuffer,CL_TRUE, 0, sizeof(float) * output.count, output.content.get() );
//            
//        } catch (cl::Error error) {
//            std::cerr << "ERROR :" << error.what() << " (" << error.err() << ")\n";
//            std::cerr << getCLErrorString(error.err()) << std::endl;
//            exit(EXIT_FAILURE);
//        }
//    }
//
//    
//    
//    
//};
