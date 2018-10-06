#include <math.h>
#include <list>
#include <vector>
#include <set>

namespace brahand {

    template <class AnyObject>
    ImageWrapper<AnyObject> ImageWrapper<AnyObject>::edges(ArrayPointer<AnyObject> isovalues) const{
        ImageWrapper<AnyObject> output(this->size, 0);
        
        brahand::uint index, neighborIndex, dx, dy, dz,  targetIndex;
        AnyObject current, neighbor, min, max;
        
        short x, y, z, a, b, c;
        short zinit = (output.size.depth == 1)? 0 : 1;
        short zlimit = (output.size.depth == 1)? 1 : output.size.depth-1;
        
        for (z = zinit ; z < zlimit ; ++z){
            short cinit = (output.size.depth == 1)? 0 : z-1;
            short climit = (output.size.depth == 1)? 0 : z+1;
            
            for ( y = 1 ; y < output.size.height-1 ; ++y){
                for ( x = 1 ; x < output.size.width-1 ; ++x){
                    index   = (z * this->size.width  * this->size.height) + (y * this->size.width) + x;
                    current = this->imagePointer[index];
                    
                    for ( c = cinit; c <= climit; c++ ){
                        for ( a = y-1; a <= y+1; a++ ){
                            for ( b = x-1; b <= x+1; b++ ){
                                if ( a == b && b == c ) continue;
                                
                                neighborIndex   = (c * this->size.width  * this->size.height) + (a * this->size.width) + b;
                                neighbor        = this->imagePointer[neighborIndex];
                                
                                min = (current <= neighbor)? current : neighbor;
                                max = (current <= neighbor)? neighbor : current;
                                
                                for(uint i = 0 ; i < isovalues.count ; ++i){
                                    
                                    if ( min <= isovalues[i] && isovalues[i] <= max ) {
                                        dy = (y+a)/2.0;	dx = (x+b)/2.0; dz = (z+c)/2.0;
                                        
                                        targetIndex = (dz * this->size.width  * this->size.height) + (dy * this->size.width) + dx;

                                        output.imagePointer[targetIndex] = (AnyObject) 255;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        
        return output;
    }
    
    template <class AnyObject>
    std::pair<ImageWrapper<AnyObject>,float> ImageWrapper<AnyObject>::jacobianSkeleton(brahand::ImageWrapper<float> &dt,brahand::ImageWrapper<float> &fluxImage, float , float fluxThreshold)
    {
        CImg<float> cimgEdges(size.width,size.height,size.depth,1,0);
        dt = ImageWrapper<float>(size);
        fluxImage = ImageWrapper<float>(size);
        
        for(uint k = 0 ; k < size.total() ; ++k){
            auto c = size(k);
            if(imagePointer[k] == 255){ cimgEdges(c.x, c.y, c.z) = 0; }
            else { cimgEdges(c.x, c.y, c.z) = 255; }
        }
        
        CImg<float> distance = cimgEdges.get_distance(0);
        
#pragma omp parallel for
        for(uint i = 0 ; i < size.total() ; ++i){
            auto c = size(i);
            dt.imagePointer[i] = distance(c.x, c.y, c.z);
        }
        
        CImgList<float> grad = distance.get_gradient("xyz");
        
        ArrayPointer<float> gradX(size.total());
        ArrayPointer<float> gradY(size.total());
        ArrayPointer<float> gradZ(size.total());
        
#pragma omp parallel for
        for(int i = 0 ; i < gradX.count ; ++i) {
            auto c = size(i);
            gradX[i] = grad(0, c.x, c.y, c.z);
            gradY[i] = grad(1, c.x, c.y, c.z);
            gradZ[i] = grad(2, c.x, c.y, c.z);
        }
        
#ifdef DEBUG_MODE
        if(size.depth <= 1){
            grad[0].normalize(0,255).save( RESULTS_PATH "gradX.png" );
            grad[1].normalize(0,255).save( RESULTS_PATH "gradY.png" );
        }
#endif
        
        //// -------------------------------------------- FLUX ----
        
        ArrayPointer<float> flux(size.total());
        
#pragma omp parallel for
        for(uint i = 0 ; i < size.total() ; ++i){
            auto c = size(i);
            auto x = c.x; auto y = c.y; auto z = c.z;
            
            if (imagePointer[size(x,y,z)] == 0){
                int stop = 0;
                float f = 0.0;
                int count = 0;
                
                for (int k = -1; k<=1; ++k)
                    for (int l = -1; l<= 1; ++l)
                        for (int m = -1; m<= 1; ++m) {
                            if (stop==1) continue;
                            
                            // Protection
                            if ((x + k<0) || (x + k>=size.width) || (y + l<0) || (y + l>=size.height) ||
                                (z + m<0) || (z + m>=size.depth) || (k==0 && l==0 && m==0)) continue;
                            ++count;
                            
                            // Test if the point is in the interior
                            if (imagePointer[size(x + k,y + l,z + m)]==255) { stop = 1; continue; }
                            
                            // Compute the flux
                            float gx = gradX[size(x + k,y + l,z + m)];
                            float gy = gradY[size(x + k,y + l,z + m)];
                            float gz = gradZ[size(x + k,y + l,z + m)];
                            
                            f+=(float)(gx*(float)k + gy*(float)l/1.0 + gz*(float)m/1.0) / std::sqrt((float)(k*k + l*l + m*m));
                        }
                
                // Update
                if (stop==1 || count==0) flux[size(x,y,z)] = 0;
                else flux[size(x,y,z)] = f/(float)count;
            }
        }
        
        //// --------------------------------------------
        
        //#ifdef DEBUG_MODE
        //        printf("\n>> Distances: %f %f\n", distance.min(), distance.max());
        //        printf("\nFlux: min: %f\t max:%f\n", flux.minMax().first, flux.minMax().second);
        //#endif
        
        fluxImage.imagePointer = flux;
        ImageWrapper<AnyObject> skeletonImage(size, 0);
        
        auto minMax = flux.minMax();
        
       
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
    
    
    //
    //    template <class AnyObject>
    //    ImageWrapper<float> ImageWrapper<AnyObject>::fastMarchingMethod(ImageWrapper<float> & edgeDensity, IndicesArray initialFront, uint ){
    //
    //        ImageWrapper<float> output(size);
    //
    //        struct PropagationStatus{ float distance; bool burned; float origin;};
    //        const std::vector<std::vector<int>> neighbors2DMap { {-1,0,0}, {0,-1,0},{0,1,0}, {1,0,0} };
    //        const std::vector<std::vector<int>> neighbors3DMap { {-1,0,0}, {0,-1,0},{0,1,0}, {1,0,0}, {0,0,-1}, {0,0,1} };
    //
    //        ArrayPointer<PropagationStatus> fire(size.total());
    //
    //        std::vector<uint> currentFront;
    //
    //        /// Build environment
    //#pragma omp parallel for
    //        for( uint index = 0 ; index < size.total() ; ++index  ){
    //            fire[index] = PropagationStatus{-1.0, false, 0.0};
    //        }
    //
    //        /// Set initial front to zero
    //        std::vector<uint> front;
    //
    //#pragma omp parallel for
    //        for( uint i = 0 ; i < initialFront.count ; ++i ){
    //            uint index = initialFront[i];
    //            fire[index].distance = 0.0;
    //            fire[index].burned = true;
    //            fire[index].origin = index;
    //#pragma omp critical
    //            front.push_back(index);
    //        }
    //        currentFront = front;
    //
    //        std::vector<std::vector<int>> map =  (size.depth == 1)?neighbors2DMap :neighbors3DMap;
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
    //                    if( nx >= 0 && nx < size.width &&
    //                       ny >= 0 && ny < size.height &&
    //                       nz >= 0 && nz < size.depth){
    //
    //                        uint nindex = size(nx,ny,nz);//  (nz * size.width  * size.height) + (ny * size.width) + nx;
    //
    //                        if( ! fire[nindex].burned ){
    //                            fire[nindex].distance = time;
    //                            fire[nindex].burned = true;
    //                            fire[nindex].origin = fire[currentFront[it]].origin;
    //#pragma omp critical
    //                            nextFront.push_back(nindex);
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
    //            edgeDensity.imagePointer[i] = originDensity + fire[i].distance;
    //            output.imagePointer[i] = fire[i].distance;
    //        }
    //
    //
    //        //        for(uint i = 0 ; i < fire.count ; ++i){ output.imagePointer[i] = fire[i].distance; }
    //        return output;
    //    }
    
    template <class AnyObject>
    float ImageWrapper<AnyObject>::propagation(ImageWrapper<float> & edgeDensity,ImageWrapper<float> dt, IndicesArray initialFront, uint){
        
        struct PropagationStatus{ bool burned; float origin;};
        const std::vector<std::vector<int>> neighbors2DMap { {-1,0,0}, {0,-1,0},{0,1,0}, {1,0,0} };
//        const std::vector<std::vector<int>> neighbors2DMap {    {-1,0,0}, {0,-1,0},{0,1,0}, {1,0,0},
//            {-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0} };
        const std::vector<std::vector<int>> neighbors3DMap { {-1,0,0}, {0,-1,0},{0,1,0}, {1,0,0}, {0,0,-1}, {0,0,1} };
        
        ArrayPointer<PropagationStatus> fire(dt.size.total());
        
        std::vector<uint> currentFront;
        
        /// Build environment
#pragma omp parallel for
        for( uint index = 0 ; index < dt.size.total() ; ++index  ){
            fire[index] = PropagationStatus{false, 0.0};
        }
        
        /// Set initial front to zero
        std::vector<uint> front;
        
#pragma omp parallel for
        for( uint i = 0 ; i < initialFront.count ; ++i ){
            uint index = initialFront[i];
            fire[index].burned = true;
            fire[index].origin = index;
#pragma omp critical
            front.push_back(index);
        }
        currentFront = front;
      
        std::vector<std::vector<int>> map =  (dt.size.depth == 1)?neighbors2DMap :neighbors3DMap;
        
        /// Expand initial front
        float time = 1.0;
        for(  ; currentFront.size() > 0 ; ++time ){
            
            std::vector<uint> nextFront;
#pragma omp parallel for
            for(uint it = 0 ; it < currentFront.size() ; ++it){
                auto coordinate = edgeDensity.size(currentFront[it]);
                
//                brahand::uint maxNeighborIndex;
//                brahand::= std::numeric_limits<AnyObject>::min();
                
                for( auto neighborsMap :  map ){
                    short nx = coordinate.x + (short)(neighborsMap[0]);
                    short ny = coordinate.y + (short)(neighborsMap[1]);
                    short nz = coordinate.z + (short)(neighborsMap[2]);
                    
                    if( nx >= 0 && nx < dt.size.width &&
                       ny >= 0 && ny < dt.size.height &&
                       nz >= 0 && nz < dt.size.depth){
                        
                        
                        uint nindex = dt.size(nx,ny,nz);//  (nz * size.width  * size.height) + (ny * size.width) + nx;
                        
                        if( ! fire[nindex].burned ){
#pragma omp critical
                            {
                                fire[nindex].burned = true;
                                fire[nindex].origin = fire[currentFront[it]].origin;
                                
                                nextFront.push_back(nindex);
                            }
                        }
                    }
                }
            }
            currentFront = nextFront;
        }
        
        
#pragma omp parallel for
        for( uint i = 0 ; i < fire.count ; ++i){
            uint originIndex = fire[i].origin;
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
        
        
        return time;
    }
}



