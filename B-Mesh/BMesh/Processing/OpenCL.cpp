//
//  OpenCL.cpp
//  BImage
//
//  Created by Bryan Gonzales Vega on 5/20/16.
//  Copyright © 2016 Bryan Gonzales Vega. All rights reserved.
//


#include "OpenCL/Utilities.cpp"
#include <string>
#include <list>

namespace brahand {

    template <class AnyObject>
    ImageWrapper<AnyObject> ImageWrapper<AnyObject>::edges(ArrayPointer<AnyObject> isovalues) const {
        ImageWrapper<AnyObject> output(this->size);

        const std::string kernelLocation = std::string(KERNELS_PATH) + "edges.cl";
        try {
            cl::Context context     = createCLContext(CL_DEVICE_TYPE_GPU, VENDOR_ANY);
            auto devices            = context.getInfo<CL_CONTEXT_DEVICES>();
            cl::CommandQueue queue  = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
            cl::Program program     = buildProgramFromSource(context, kernelLocation.c_str(), "-D ISOVALUES_COUNT=" + std::to_string(isovalues.count));
            cl::Kernel kernel       = cl::Kernel(program, "edges");

            cl::Buffer inputBuffer( context, CL_MEM_USE_HOST_PTR, sizeof(brahand::PixelType) * imagePointer.count, imagePointer.content.get() );
            cl::Buffer isovaluesBuffer( context, CL_MEM_USE_HOST_PTR, sizeof(brahand::PixelType) * isovalues.count , isovalues.content.get());
            cl::Buffer outputBuffer( context, CL_MEM_WRITE_ONLY, sizeof(brahand::PixelType) * output.imagePointer.count);

            kernel.setArg(0, inputBuffer);
            kernel.setArg(1, isovaluesBuffer);
            kernel.setArg(2, outputBuffer);

            queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(size.width, size.height, size.depth), cl::NullRange);
            queue.enqueueReadBuffer(outputBuffer,CL_TRUE, 0, sizeof(brahand::PixelType) * imagePointer.count, output.imagePointer.content.get() );
            queue.finish();
        } catch(cl::Error error) {
            std::cout << error.what() << "(" << error.err() << ")" << std::endl;
            std::cout << getCLErrorString(error.err()) << std::endl;
            exit(EXIT_FAILURE);
        }

        return output;
    }


    template <class AnyObject>
    std::pair<ImageWrapper<AnyObject>,float> ImageWrapper<AnyObject>::jacobianSkeleton(ImageWrapper<float> &dt, ImageWrapper<float> &fluxImage ,float , float fluxThreshold) {
        CImg<float> cimgEdges(size.width,size.height,size.depth,1,0);

        dt = ImageWrapper<float>(size);
        fluxImage = ImageWrapper<float>(size);
#pragma omp parallel for
        for(uint k = 0 ; k < size.total() ; ++k){
            auto c = size(k);
            if(imagePointer[k] == 255){ cimgEdges(c.x, c.y, c.z) = 0; }
            else { cimgEdges(c.x, c.y, c.z) = 255; }
        }

        CImg<float> distance = cimgEdges.get_distance(0); // Faster than KDTree

#pragma omp parallel for
        for(uint i = 0 ; i < size.total() ; ++i){
            auto c = size(i);
            dt.imagePointer[i] = distance(c.x, c.y, c.z);
        }

        CImgList<float> grad = distance.get_gradient("xyz");

        struct GradientElement{ float x; float y; float z; };
        ArrayPointer<GradientElement> gradient(size.total());

#pragma omp parallel for
        for(int i = 0 ; i < size.total() ; ++i) {
            auto c = size(i);
            gradient[i] = {grad(0, c.x, c.y, c.z), grad(1, c.x, c.y, c.z),  grad(2, c.x, c.y, c.z)};
        }
        
#ifdef DEBUG_MODE
        if(size.depth <= 1){
            grad(0).normalize(0,255).save( RESULTS_PATH "gradX.png" );
            grad(1).normalize(0,255).save( RESULTS_PATH "gradY.png" );
        }
#endif

        //// -------------------------------------------- FLUX ----
        ArrayPointer<float> flux(size.total());
        const std::string kernelLocation = std::string(KERNELS_PATH) + "skeleton.cl";
        try {
            cl::Context context     = createCLContext(CL_DEVICE_TYPE_GPU, VENDOR_ANY);
            auto devices            = context.getInfo<CL_CONTEXT_DEVICES>();
            cl::CommandQueue queue  = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
            cl::Program program     = buildProgramFromSource(context, kernelLocation.c_str());
            cl::Kernel kernel       = cl::Kernel(program, "flux");

            cl::Buffer inputBuffer( context, CL_MEM_USE_HOST_PTR, sizeof(short) * size.total(), imagePointer.content.get() );
            cl::Buffer gradientBuffer( context, CL_MEM_USE_HOST_PTR, sizeof(GradientElement) * gradient.count, gradient.content.get() );
            cl::Buffer outputBuffer( context, CL_MEM_WRITE_ONLY, sizeof(float) * size.total());

            kernel.setArg(0, inputBuffer);
            kernel.setArg(1, gradientBuffer);
            kernel.setArg(2, outputBuffer);

            queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(size.width, size.height, size.depth), cl::NullRange);
            queue.enqueueReadBuffer(outputBuffer,CL_TRUE, 0, sizeof(float) * size.total(), flux.content.get() );
            queue.finish();
        } catch(cl::Error error) {
            std::cout << error.what() << "(" << error.err() << ")" << std::endl;
            std::cout << getCLErrorString(error.err()) << std::endl;
            exit(EXIT_FAILURE);
        }

        //#ifdef DEBUG_MODE
        //        printf("\n>> Distances: %f %f\n", distance.min(), distance.max());
        //        printf("\nFlux: min: %f\t max:%f\n", flux.minMax().first, flux.minMax().second);
        //#endif

        fluxImage.imagePointer = flux;
        ImageWrapper<AnyObject> skeletonImage(size);

        auto minMax = flux.minMax();
        printf("Flux: %f, %f\n", minMax.first , minMax.second);
        
#pragma omp parallel for
        for(uint i = 0 ; i < size.total() ; ++i){
            if(flux[i] >= fluxThreshold){
                skeletonImage.imagePointer[i] = 0;
            } else{
                skeletonImage.imagePointer[i] = 255;
            }
        }

        return {skeletonImage, minMax.first};
    }

    template <class AnyObject>
    float ImageWrapper<AnyObject>::propagation(ImageWrapper<float> & edgeDensity,ImageWrapper<float> dt, IndicesArray initialFront, uint waveLength){

        struct PropagationStatus{ bool burned; float origin;};
        ArrayPointer<PropagationStatus> fireIn(dt.size.total());
        ArrayPointer<PropagationStatus> fireOut(dt.size.total());

        /// Build environment
#pragma omp parallel for
        for( uint index = 0 ; index < dt.size.total() ; ++index  ){
            fireIn[index] = PropagationStatus{false, 0.0};
            fireOut[index] = PropagationStatus{false, 0.0};
        }

#pragma omp parallel for
        for( uint i = 0 ; i < initialFront.count ; ++i ){
            uint index = initialFront[i];
            fireIn[index].burned = true;
            fireIn[index].origin = index;
        }

        const std::string kernelLocation = std::string(KERNELS_PATH) + "distanceTransform.cl";
        try {
            cl::Context context     = createCLContext(CL_DEVICE_TYPE_GPU, VENDOR_ANY);
            auto devices            = context.getInfo<CL_CONTEXT_DEVICES>();
            cl::CommandQueue queue  = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
            cl::Program program     = buildProgramFromSource(context, kernelLocation.c_str(),"-D EDGES_COUNT=" + std::to_string(initialFront.count));
            cl::Kernel initKernel   = cl::Kernel(program, "init");
            cl::Kernel kernel       = cl::Kernel(program, "fastMarchingMethod");

            cl::Buffer inputBuffer( context, CL_MEM_USE_HOST_PTR, sizeof(PropagationStatus) * dt.size.total(), fireIn.content.get() );
            cl::Buffer outputBuffer( context, CL_MEM_ALLOC_HOST_PTR, sizeof(PropagationStatus) * dt.size.total() );

            initKernel.setArg(0, inputBuffer);
            initKernel.setArg(1, outputBuffer);
            queue.enqueueNDRangeKernel(initKernel, cl::NullRange, cl::NDRange(dt.size.width, dt.size.height, dt.size.depth), cl::NullRange);
            queue.finish();

            for(auto k = 0 ; k < waveLength ; ++k){
                if(k % 2 == 0) {
                    kernel.setArg(0, inputBuffer);
                    kernel.setArg(1, outputBuffer);
                } else {
                    kernel.setArg(0, outputBuffer);
                    kernel.setArg(1, inputBuffer);
                }
                queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(dt.size.width, dt.size.height, dt.size.depth), cl::NullRange);
            }
            queue.finish();

            queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, sizeof(PropagationStatus) * dt.size.total(), fireOut.content.get() );

        } catch(cl::Error error) {
            std::cout << error.what() << "(" << error.err() << ")" << std::endl;
            std::cout << getCLErrorString(error.err()) << std::endl;
            exit(EXIT_FAILURE);
        }

#pragma omp parallel for
        for( uint i = 0 ; i < fireOut.count ; ++i){
            uint originIndex = fireOut[i].origin;
            float originDensity = edgeDensity.imagePointer[originIndex];
            edgeDensity.imagePointer[i] = originDensity + dt.imagePointer[i];
        }

#ifdef DEBUG_MODE
        CImg<float> eDensity(edgeDensity.size.width,edgeDensity.size.height,edgeDensity.size.depth,1,0);
        for(uint k = 0 ; k < edgeDensity.size.total() ; ++k){
            auto c = edgeDensity.size(k);
            eDensity(c.x,c.y,c.z) = edgeDensity.imagePointer[k];
        }
        eDensity.blur(8);
        
        for(uint k = 0 ; k < edgeDensity.size.total() ; ++k){
            auto c = edgeDensity.size(k);
            edgeDensity.imagePointer[k] = eDensity(c.x,c.y,c.z);
        }
#endif

        return waveLength;
//        struct PropagationStatus{ bool burned; float origin;};
//        const std::vector<std::vector<int>> neighbors2DMap { {-1,0,0}, {0,-1,0},{0,1,0}, {1,0,0} };
//        const std::vector<std::vector<int>> neighbors3DMap { {-1,0,0}, {0,-1,0},{0,1,0}, {1,0,0}, {0,0,-1}, {0,0,1} };
//
//        ArrayPointer<PropagationStatus> fire(dt.size.total());
//
//        std::vector<uint> currentFront;
//
//        /// Build environment
//#pragma omp parallel for
//        for( uint index = 0 ; index < dt.size.total() ; ++index  ){
//            fire[index] = PropagationStatus{false, 0.0};
//        }
//
//        /// Set initial front to zero
//        std::vector<uint> front;
//
//#pragma omp parallel for
//        for( uint i = 0 ; i < initialFront.count ; ++i ){
//            uint index = initialFront[i];
//            fire[index].burned = true;
//            fire[index].origin = index;
//#pragma omp critical
//            front.push_back(index);
//        }
//        currentFront = front;
//
//        std::vector<std::vector<int>> map =  (dt.size.depth == 1)?neighbors2DMap :neighbors3DMap;
//
//        /// Expand initial front
//        float time = 1.0;
//        for(  ; currentFront.size() > 0 ; ++time ){
//
//            std::vector<uint> nextFront;
//#pragma omp parallel for
//            for(uint it = 0 ; it < currentFront.size() ; ++it){
//                auto coordinate = edgeDensity.size(currentFront[it]);
//                for( auto neighborsMap :  map ){
//                    short nx = coordinate.x + (short)(neighborsMap[0]);
//                    short ny = coordinate.y + (short)(neighborsMap[1]);
//                    short nz = coordinate.z + (short)(neighborsMap[2]);
//
//                    if( nx >= 0 && nx < dt.size.width &&
//                       ny >= 0 && ny < dt.size.height &&
//                       nz >= 0 && nz < dt.size.depth){
//
//                        uint nindex = dt.size(nx,ny,nz);//  (nz * size.width  * size.height) + (ny * size.width) + nx;
//
//                        if( ! fire[nindex].burned ){
//                            #pragma omp critical
//                            {
//                            fire[nindex].burned = true;
//                            fire[nindex].origin = fire[currentFront[it]].origin;
//
//                            nextFront.push_back(nindex);
//                            }
//                        }
//                    }
//                }
//            }
//            currentFront = nextFront;
//        }
//
//#pragma omp parallel for
//        for( uint i = 0 ; i < fire.count ; ++i){
//            uint originIndex = fire[i].origin;
//            float originDensity = edgeDensity.imagePointer[originIndex];
//            edgeDensity.imagePointer[i] = originDensity + dt.imagePointer[i];
//        }
    }


//    template <class AnyObject>
//    ImageWrapper<float> ImageWrapper<AnyObject>::fastMarchingMethod(ImageWrapper<float> & edgeDensity, IndicesArray initialFront, uint waveLength){
//
//        ImageWrapper<float> output(size, 0.0);
//
//        struct PropagationStatus{ float distance; bool burned; float origin;};
//        ArrayPointer<PropagationStatus> fireIn(size.total());
//        ArrayPointer<PropagationStatus> fireOut(size.total());
//
//        /// Build environment
//#pragma omp parallel for
//        for( uint index = 0 ; index < size.total() ; ++index  ){
//            fireIn[index] = PropagationStatus{0.0, false, 0.0};
//            fireOut[index] = PropagationStatus{0.0, false, 0.0};
//        }
//
//#pragma omp parallel for
//        for( uint i = 0 ; i < initialFront.count ; ++i ){
//            uint index = initialFront[i];
//            fireIn[index].distance = 1.0;
//            fireIn[index].burned = true;
//            fireIn[index].origin = index;
//        }
//
//        try {
//            cl::Context context     = createCLContext(CL_DEVICE_TYPE_GPU, VENDOR_ANY);
//            auto devices            = context.getInfo<CL_CONTEXT_DEVICES>();
//            cl::CommandQueue queue  = cl::CommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE);
//            cl::Program program     = buildProgramFromSource(context, "distanceTransform.cl","-D EDGES_COUNT=" + std::to_string(initialFront.count));
//            cl::Kernel kernel       = cl::Kernel(program, "fastMarchingMethod");
//
//            cl::Buffer inputBuffer( context, CL_MEM_USE_HOST_PTR, sizeof(PropagationStatus) * size.total(), fireIn.content.get() );
//            cl::Buffer outputBuffer( context, CL_MEM_ALLOC_HOST_PTR, sizeof(PropagationStatus) * size.total() );
//
//            for(auto k = 0 ; k < waveLength ; ++k){
//                if(k % 2 == 0) {
//                    kernel.setArg(0, inputBuffer);
//                    kernel.setArg(1, outputBuffer);
//                } else {
//                    kernel.setArg(0, outputBuffer);
//                    kernel.setArg(1, inputBuffer);
//                }
//                queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(size.width, size.height, size.depth), cl::NullRange);
//            }
//            queue.finish();
//
//            queue.enqueueReadBuffer(outputBuffer, CL_TRUE, 0, sizeof(PropagationStatus) * size.total(), fireOut.content.get() );
//
//        } catch(cl::Error error) {
//            std::cout << error.what() << "(" << error.err() << ")" << std::endl;
//            std::cout << getCLErrorString(error.err()) << std::endl;
//            exit(EXIT_FAILURE);
//        }
//
//#pragma omp parallel for
//        for( uint i = 0 ; i < fireOut.count ; ++i){
//            uint originIndex = fireOut[i].origin;
//            float originDensity = edgeDensity.imagePointer[originIndex];
//            edgeDensity.imagePointer[i] = originDensity + fireOut[i].distance;
//            output.imagePointer[i] = fireOut[i].distance;
//        }
//
//        return output;
//    }

}
