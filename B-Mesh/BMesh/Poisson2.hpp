#ifndef Poisson_hpp
#define Poisson_hpp

#include "Modules/Essentials/Essentials.hpp"

#include <random>
#include <iterator>
#include <algorithm>
#include <iostream>

#include <stdlib.h>
#include <time.h>
#include <set>
#include <random>

namespace brahand {
    
    enum class PoissonStatus{ IDLE, ACTIVE, ACCEPTED, REJECTED };
    
    struct PoissonSample {
        float priority;
        ImageCoordinate position;
        PoissonStatus status;
        int thread;
        bool enabled;
    };
    
    template<class T>
    inline float euclideanDistance ( T ax, T ay, T az, T bx, T by, T bz) {
        float xx = (ax - bx) * (ax - bx);
        float yy = (ay - by) * (ay - by);
        float zz = (az - bz) * (az - bz);
        return std::sqrt( xx + yy + zz );
    }
    
    class Poisson{
    private:
        struct Group {
            typedef struct { uint low, high; } Range;
            Range range;
            uint currentIndex;
        };
        
        std::random_device randomDevice;
        std::mt19937 mersenneTwister{randomDevice()};

        
        ImageSize size;
        uint cores = 1;
        brahand::ImageWrapper<float> density;
        std::vector<std::vector<uint>> iddlePointsByThread{cores};
        std::vector<std::vector<uint>> activePointsByThread{cores};
        std::vector<Group> groups{cores};
        std::vector<uint> shuffledIndices;
        std::vector<PoissonSample> points;
        
        std::vector<uint> setupGroupsIndex(IndicesArray indices){
            std::vector<uint> shuffledIndices; shuffledIndices.reserve(indices.count);
            for(uint i = 0 ; i < indices.count ; ++i){ shuffledIndices.push_back( indices[i] ); }

            std::shuffle(shuffledIndices.begin(), shuffledIndices.end(), mersenneTwister);
            
            for (uint i = 1 ; i <= cores ; ++i) {
                groups[i-1].range = {   (uint)((float)(i - 1) / (float) cores * (float)indices.count),
                                        (uint)((float)i / (float)cores * (float)indices.count) - 1 };

                groups[i-1].currentIndex =  (uint) ((float)(i - 1) / (float) cores * (float) indices.count);
//                printf("%d: [%d %d], %d\n", i, groups[i-1].range.low, groups[i-1].range.high, groups[i-1].currentIndex);
            }
            
            return shuffledIndices;
        }
        
        std::pair<std::vector<uint>,std::vector<uint>> findActiveAndIdleNeighbors(uint index, float maskSize){
            std::vector<uint> activePoints, idlePoints;
            auto c = size(index);
            
            for(int k = 0 ; k < ((size.depth > 1)?maskSize:1) ; ++k){
                for(int m = 0 ; m < maskSize ; ++m) {
                    for(int n = 0 ; n < maskSize ; ++n){
                        int idx = c.x - floor(maskSize/2.0) + m;
                        int idy = c.y - floor(maskSize/2.0) + n;
                        int idz = c.z - floor( ((size.depth > 1)?maskSize:1) /2.0) + k;
                        
                        if( idx >= 0 && idx < size.width && idy >= 0 && idy < size.height && idz >= 0 && idz < size.depth ){
                            float d = euclideanDistance<float>((float)idx, (float)idy, (float)idz,(float)c.x, (float)c.y, (float)c.z);
                            auto neighborIndex = size(idx, idy, idz);

                            auto radius = std::max(density.imagePointer[index], density.imagePointer[neighborIndex] );
                            
                            if(points[neighborIndex].status == PoissonStatus::ACTIVE){
                                if(  points[neighborIndex].enabled == true &&  d <= radius && neighborIndex != index ){
                                    activePoints.push_back( neighborIndex );
                                }
                            } else if(points[neighborIndex].status == PoissonStatus::IDLE){
                                if(  d <= radius && neighborIndex != index ){
                                    idlePoints.push_back( neighborIndex );
                                }
                            }
                        }
                    }
                }
            }

            return {activePoints, idlePoints};
        }
        
        std::pair<bool, std::vector<uint>> findNeighbors(uint index, float maskSize, PoissonStatus status){
            std::vector<uint> neighbors;
            auto c = size(index);
            
            bool matchStatus = false;
            for(int k = 0 ; k < ((size.depth > 1)?maskSize:1) ; ++k){
                for(int m = 0 ; m < maskSize ; ++m) {
                    for(int n = 0 ; n < maskSize ; ++n){
                        int idx = c.x - floor(maskSize/2.0) + m;
                        int idy = c.y - floor(maskSize/2.0) + n;
                        int idz = c.z - floor( ((size.depth > 1)?maskSize:1) /2.0) + k;
                        
                        if( idx >= 0 && idx < size.width && idy >= 0 && idy < size.height && idz >= 0 && idz < size.depth ){
                            float d = euclideanDistance<float>((float)idx, (float)idy, (float)idz,(float)c.x, (float)c.y, (float)c.z);
                            auto neighborIndex = size(idx, idy, idz);
                            
                            auto radius = std::max(density.imagePointer[index], density.imagePointer[neighborIndex] );
                            
                            if(  d <= radius && neighborIndex != index ){
                                neighbors.push_back( neighborIndex );
                            }

                            if(points[neighborIndex].status == status){
                                matchStatus = true;
                            }
                        }
                    }
                }
            }
            
            return {matchStatus, neighbors};
        }
        
        void checkStatus(uint index ){
            
            if( points[index].status != PoissonStatus::ACTIVE ){
                return;
            }
            
            for(auto neighborIndex : activePointsByThread[ points[index].thread ]  ){
                if( points[index].priority < points[neighborIndex].priority ){
                    
                    checkStatus(neighborIndex);
                    
                    if(points[neighborIndex].status == PoissonStatus::ACCEPTED){
                        #pragma omp atomic write
                        points[index].status = PoissonStatus::REJECTED;
                        return;
                    }
                }
            }
            #pragma omp atomic write
            points[index].status = PoissonStatus::ACCEPTED;
        }
        
    public:
        Poisson(ImageSize size, brahand::ImageWrapper<float> density){
            this->size      = size;
            this->density   = density;

            points.reserve(size.total());
            for(uint i = 0 ; i < size.total() ; ++i) {
                points.push_back( {-1.0, size(i), PoissonStatus::IDLE, -1, false} );
                points[i].priority = ( (float)size(points[i].position.x, points[i].position.y, points[i].position.z) / (float)points.size() );
            }
        }
        
        IndicesArray sampling( IndicesArray enabledPoints ){
            shuffledIndices = setupGroupsIndex(enabledPoints);
            assert(shuffledIndices.size() == enabledPoints.count);
            
            #pragma omp parallel for
            for(uint index = 0 ; index < enabledPoints.count ; ++index){
                points[enabledPoints[index]].enabled = true;
            }

            for(uint t = 0 ; t < cores ; ++t){ groups[t].currentIndex = groups[t].range.low; }
            
            bool continueFlag;
            do{
                continueFlag = false;
                
                #pragma omp parallel for
                for(uint t = 0 ; t < cores ; ++t){
                    while(groups[t].currentIndex <= groups[t].range.high &&
                          points[shuffledIndices[groups[t].currentIndex]].status != PoissonStatus::IDLE )
                    {
                        groups[t].currentIndex++;
                    }
                    
                    if(groups[t].currentIndex > groups[t].range.high){
                        groups[t].currentIndex = groups[t].range.high;
                    }
                    
                    assert(points[shuffledIndices[groups[t].currentIndex]].enabled);
                    
                    points[shuffledIndices[groups[t].currentIndex]].status  = PoissonStatus::ACTIVE;
                    points[shuffledIndices[groups[t].currentIndex]].thread  = t;
                }
                
                /// Collect active and idle points per thread
                
                #pragma omp parallel for
                for(uint t = 0 ; t < cores ; ++t){
                    float radius = density.imagePointer[shuffledIndices[groups[t].currentIndex]];
                    auto neighbors = findActiveAndIdleNeighbors(shuffledIndices[groups[t].currentIndex], (radius*2)+1 );
                    activePointsByThread[t] = neighbors.first;
                    iddlePointsByThread[t]  = neighbors.second;
                }
                
                #pragma omp parallel for
                for(uint t = 0 ; t < cores ; ++t){
                    checkStatus(shuffledIndices[groups[t].currentIndex]);
                    
                    if(points[shuffledIndices[groups[t].currentIndex]].status == PoissonStatus::ACCEPTED){
                        for(auto idleIndex : iddlePointsByThread[t] ){
                            #pragma omp atomic write
                            points[idleIndex].status = PoissonStatus::REJECTED;
                        }
                    }
                }

                for(uint t = 0 ; t < cores ; ++t){
                    for (uint inc = groups[t].currentIndex ;  inc < groups[t].range.high ; ++inc) {
                        if(  points[inc].status == PoissonStatus::IDLE ){
                            continueFlag = true;
                            break;
                        }
                    }
                }
            } while(continueFlag);
            
            std::vector<uint> acceptedSamples;
            int pcounter = 0;
            for(auto point : points){
                if( point.status == PoissonStatus::ACCEPTED ){
                    auto c = point.position;
                    // printf("%d %d: [%d %d]\n", pcounter, size(pcounter), c.x, c.y);
                    acceptedSamples.push_back(size(c.x,c.y,c.z));
                    // std::cout << pcounter << " : " << acceptedSamples[pcounter] << std::endl;
                    pcounter++;
                }
            }
            
            return {acceptedSamples};
        }
        
        IndicesArray maximal(IndicesArray edge, IndicesArray samples ){
            int count = 0;
            for(uint index = 0 ; index < edge.count ; ++index){
                uint sampleIndex = edge[index];
                if( points[sampleIndex].status != PoissonStatus::ACCEPTED ){
                    float radius = density.imagePointer[sampleIndex];
                    auto query = findNeighbors(sampleIndex, radius, PoissonStatus::ACCEPTED);
                    if(query.first == false){
                        for( auto nIndex : query.second ){
                            points[nIndex].status = PoissonStatus::REJECTED;
                        }
                        points[sampleIndex].status = PoissonStatus::ACCEPTED;
                        count++;
                    }
                }
            }
            
            std::cout << ">> " << edge.count << " " << count << "\t" << samples.count << "\n";
            
            std::vector<uint> maximalSamples;
            for(uint index = 0 ; index < edge.count ; ++index){
                uint sampleIndex = edge[index];
                if( points[sampleIndex].status == PoissonStatus::ACCEPTED ){
                    maximalSamples.push_back(sampleIndex);
                }
            }

            return {maximalSamples};
        }
        
    };
}

#endif /* Poisson_hpp */
