#ifndef Image_hpp
#define Image_hpp

#ifndef IMAGEMAGICK_PATH
#error Macro IMAGEMAGICK_PATH is required!
#endif

#include <fstream>
#include <queue> // skeleton
#include <list>

#include "Modules/Essentials/Essentials.hpp"
#include "Modules/Essentials/FileManager.hpp"

#include "Modules/KdTree/include/nanoflann.hpp"
using namespace nanoflann;

//#define cimg_use_openmp
#define cimg_display 0
#define cimg_plugin "plugins/skeleton.h"
#include "Modules/CImg/CImg.h"
using namespace cimg_library;

//#if defined(__APPLE__) || defined(__MACOSX)
//#include <libiomp/omp.h>
//#else
//#include <omp.h>
//#endif

//#include "Exporter.hpp"

namespace brahand {
    
    typedef CoordinateWrapper<short> ImageCoordinate;
    typedef CoordinateWrapper<float> EuclideanCoordinate;
    
    struct ImageSize {
        ushort width, height, depth;
        
        ImageSize() = default;
        ImageSize(ushort w, ushort h, ushort d=1) : width(w), height(h), depth(d){}
        inline uint total() const { return width * height * depth; }
        
        inline uint operator ()(ushort x, ushort y, ushort z){
            return (z * width * height) + (y * width) + x;
        }
        
        inline ImageCoordinate operator() (uint index){
            short layer = index / (width * height);
            index -= (layer * width * height);
            short row = index / width;
            short col = index % width;
            
            return {col, row, layer};
        }
    };
    
    template <typename T>
    struct PointCloud {
        struct Point {
            T  x,y,z;
        };
        
        std::vector<Point>  pts;
        
        inline size_t kdtree_get_point_count() const { return pts.size(); }
        
        // Returns the distance between the vector "p1[0:size-1]" and the data point with index "idx_p2" stored in the class:
        inline T kdtree_distance(const T *p1, const size_t idx_p2,size_t /*size*/) const {
            const T d0=p1[0]-pts[idx_p2].x;
            const T d1=p1[1]-pts[idx_p2].y;
            const T d2=p1[2]-pts[idx_p2].z;
            return d0*d0+d1*d1+d2*d2;
        }
        
        // Returns the dim'th component of the idx'th point in the class:
        // Since this is inlined and the "dim" argument is typically an immediate value, the
        //  "if/else's" are actually solved at compile time.
        inline T kdtree_get_pt(const size_t idx, int dim) const {
            if (dim==0) return pts[idx].x;
            else if (dim==1) return pts[idx].y;
            else return pts[idx].z;
        }
        
        // Optional bounding-box computation: return false to default to a standard bbox computation loop.
        //   Return true if the BBOX was already computed by the class and returned in "bb" so it can be avoided to redo it again.
        //   Look at bb.size() to find out the expected dimensionality (e.g. 2 or 3 for point clouds)
        template <class BBOX>
        bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }
    };
    
    
#pragma mark - ImageWrapper<AnyObject>
    
    template <class AnyObject>
    struct ImageWrapper {
        ImageSize size;
        ArrayPointer<AnyObject> imagePointer;
        
        ImageWrapper() = default;
        
        ImageWrapper(std::string path){
            cimg::imagemagick_path(IMAGEMAGICK_PATH);
            
            SourcesPathArray files = (path.find_last_of('.') == 0)?brahand::filesInDirectory(path) : brahand::SourcesPathArray{path};
            std::vector<CImg<>> imagesList(files.size());
            
            auto firstImage = CImg<>(files[0].c_str());
            this->size = {(ushort)firstImage.width(), (ushort)firstImage.height(), (ushort)files.size()};
            this->imagePointer = ArrayPointer<AnyObject>(size.total());
            
            //#pragma omp parallel for
            for(auto z = 0 ; z < files.size() ; ++z){
                imagesList[z] = CImg<>(files[z].c_str());
                if(imagesList[z].spectrum() == 3){
                    imagesList[z] = imagesList[z].get_RGBtoYCbCr().get_channel(0);
                }
                cimg_forXY(imagesList[z], x, y){
                    this->imagePointer[this->size(x, y, z)] = (AnyObject)imagesList[z](x,y);
                }
            }

            // auto image = CImg<>(files[0].c_str());
            // this->size = {(ushort)image.width(), (ushort)image.height(), (ushort)image.height()};
            // this->imagePointer = ArrayPointer<AnyObject>(size.total());
            
            // //#pragma omp parallel for
            // for(auto z = 0 ; z < files.size() ; ++z){
            //     imagesList[z] = CImg<>(files[z].c_str());
            //     if(imagesList[z].spectrum() == 3){
            //         imagesList[z] = imagesList[z].get_RGBtoYCbCr().get_channel(0);
            //     }
            //     cimg_forXY(imagesList[z], x, y){
            //         this->imagePointer[this->size(x, y, z)] = (AnyObject)imagesList[z](x,y);
            //     }
            // }
        }
        ImageWrapper(ImageSize size){
            this->size = size;
            this->imagePointer = ArrayPointer<AnyObject>{size.total()};
        }
        ImageWrapper(ImageSize size, AnyObject defaultValue){
            this->size = size;
            this->imagePointer = ArrayPointer<AnyObject>{size.total(), defaultValue};
        }

        void save(std::string filename){
            // Create a Float type image...
            CImg<AnyObject> image(size.width, size.height, size.depth, 1, 0);

            for(auto z = 0 ; z < size.depth ; ++z){
                for(auto y = 0 ; y < size.height ; ++y){
                    for(auto x = 0 ; x < size.width ; ++x){
                        image(x,y,z) = this->imagePointer[this->size(x, y, z)];
                    }
                }
            }

            image.save(filename.c_str());
        }
        
#pragma mark - OpenMP/OpenCL processing functions
        
        ImageWrapper<AnyObject> edges(ArrayPointer<AnyObject> isovalues) const;
        
        std::pair<ImageWrapper<AnyObject>,float> jacobianSkeleton(brahand::ImageWrapper<float> &dt,brahand::ImageWrapper<float> &fluxImage,float blurSigma, float fluxThreshold);
        static float propagation(ImageWrapper<float> & density, ImageWrapper<float> dt, IndicesArray, uint);
        
        static float propagation_keynote(ImageWrapper<float> & density, IndicesArray, uint, std::string path);
        
        
#pragma mark - Sequential processing functions
        
        template <class OtherObject>
        ImageWrapper<OtherObject> normalize(OtherObject normMin, OtherObject normMax ){
            
            ImageWrapper<OtherObject> newImage(size);
            
            auto minMax = imagePointer.minMax();
            AnyObject min = minMax.first, max = minMax.second;
            float a = (float)(normMax - normMin) / (float)(max - min), b = normMax - a * max;
            
#pragma omp parallel for
            for(uint i = 0 ; i < imagePointer.count ; ++i){
                newImage.imagePointer[i] = (OtherObject)(a * imagePointer[i] + b);
            }
            return newImage;
        }
        
        std::pair<IndicesArray,IndicesArray> features(){
            std::vector<uint> vector; vector.reserve(size.total());
            std::vector<uint> vectorNonFeatures; vectorNonFeatures.reserve(size.total());
            for(uint i = 0 ; i < imagePointer.count ; ++i){
                if(imagePointer[i] != 0){ vector.push_back( i ); }
                else{ vectorNonFeatures.push_back( i );}
            }
            return {IndicesArray{vector}, IndicesArray{vectorNonFeatures}};
        }
        
        void save2DElevationMap(std::string folder, std::string name){
            assert(size.depth == 1);
            std::string fullPath = folder + name + ".csv";
            std::ofstream file; file.open( fullPath.c_str() );
            file << "\"x\",\"y\",\"z\"\n";
            for(uint i = 0 ; i < size.total() ; ++i){
                auto c = size(i);
                file << c.x << "," << c.y << "," << imagePointer[i] << "\n";
            }
            file.close();
            
            std::string lSetfullPath = folder + name + "-levelSets.txt";
            std::ofstream levelSet; levelSet.open( lSetfullPath );
            for(uint i = 0 ; i < size.width ; ++i){
                for(uint j = 0 ; j < size.height ; ++j){
                    auto c = size(i,j,0);
                    levelSet << imagePointer[c] << " ";
                }
                levelSet << "\n";
            }
            levelSet.close();
        }
        
        brahand::IndicesArray addFrame(){
            std::vector<uint> frameCoordinates;
            for(int z = 0 ; z < size.depth ; ++z){
                for(int x = 0 ; x < size.width ; ++x){
                    for(auto y : {0, size.height-1}){
                        imagePointer[size(x,y,z)] = 255;
                        frameCoordinates.push_back(size(x,y,z));
                    }
                }
                for(int y = 0 ; y < size.height ; ++y){
                    for(auto x : {0, size.width-1}){
                        imagePointer[size(x,y,z)] = 255;
                        frameCoordinates.push_back(size(x,y,z));
                    }
                }
            }
            if(size.depth > 1){ // In 3D add sides (near,far)
                for(auto z : {0, size.depth-1})
                    for(int x = 0 ; x < size.width ; ++x)
                        for(int y = 0 ; y < size.height ; ++y){
                            imagePointer[size(x,y,z)] = 255;
                            frameCoordinates.push_back(size(x,y,z));
                        }
            }
            return {frameCoordinates};
        }
        
        
        ImageWrapper<float> edgeDensity(IndicesArray edges, IndicesArray skeleton){
            ImageWrapper<float> densityImage(size, -1);
            
            /// Insert skeleton elements into kdtree
            PointCloud<float> cloud;
            cloud.pts.resize( skeleton.count );
            
            for( uint sIndex = 0 ; sIndex < skeleton.count ; ++sIndex  ){
                auto coordinate = size(skeleton[sIndex]);
                cloud.pts[sIndex].x = (float)coordinate.x;
                cloud.pts[sIndex].y = (float)coordinate.y;
                cloud.pts[sIndex].z = (float)coordinate.z;
            }
            
            typedef KDTreeSingleIndexAdaptor< L2_Simple_Adaptor<float, PointCloud<float> > , PointCloud<float>, 3 > my_kd_tree_t;
            my_kd_tree_t tree(3, cloud, KDTreeSingleIndexAdaptorParams(10) );
            tree.buildIndex();
            
#pragma omp parallel for
            for( uint index = 0 ; index < edges.count ; ++index  ){
                auto coordinate = size(edges[index]);
                float query_pt[3] = { (float)coordinate.x, (float)coordinate.y, (float)coordinate.z};
                
                size_t ret_index; float out_dist_sqr;
                nanoflann::KNNResultSet<float> resultSet(1);
                resultSet.init(&ret_index, &out_dist_sqr );
                tree.findNeighbors(resultSet, &query_pt[0], nanoflann::SearchParams(10));
                
                float dis = (float)sqrt( out_dist_sqr );
                densityImage.imagePointer[edges[index]] = dis;
            }
            
            return densityImage;
        }
    };
    
    typedef short PixelType;
    typedef ImageWrapper<PixelType> Image;
    typedef std::vector<PixelType> ImageIsovaluesVector; // ArrayPtr conversion comes later
}

//#if PROCESSING_TYPE == OpenMP
#include "Processing/OpenMP.cpp"
//#elif PROCESSING_TYPE == OpenCL
//#include "Processing/OpenCL.cpp"
//#else
//static_assert(true, "PROCESSING_TYPE macro isn't valid");
//#endif

#endif /* Image_hpp */
