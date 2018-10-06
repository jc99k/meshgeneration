#ifndef Exporter_hpp
#define Exporter_hpp

#include "Image.hpp"
#include <string>
#include <fstream>
#include <memory>

namespace brahand {
    
    class VTKExport{
    public:
        VTKExport() = default;
    
        template <class AnyObject>
        static void grayImage( brahand::ImageWrapper<AnyObject> image, std::string folder, std::string name){
            printf("Exporting %s \t.... ", name.c_str());
            
            auto size = image.size;
            std::string fullPath = folder + name + ".vtk";
            
            std::ofstream file; file.open( fullPath.c_str() );
            file << "# vtk DataFile Version 1.0\nUnstructured Grid Example\nASCII\nDATASET UNSTRUCTURED_GRID\n";
            
            file << "POINTS " << size.total() << " float\n";
            for(ushort z = 0 ; z < size.depth ; ++z){
                for(ushort y = 0 ; y < size.height ; ++y){
                    for(ushort x = 0 ; x < size.width ; ++x){
                        file << x << " " << size.height - y << " " << z << "\n";
                    }
                }
            }
            
            file << "CELLS " << size.total() << " " << size.total() * 2 << "\n";
            for(uint i = 0 ; i < size.total() ; ++i) file << "1 " << i << "\n";
            
            file << "CELL_TYPES " << size.total() << "\n";
            for(uint i = 0 ; i < size.total() ; ++i) file << "1 ";
            
            file << "\nCELL_DATA " << size.total() << "\nCOLOR_SCALARS lut 1\n";
            for(uint i = 0 ; i < size.total() ; ++i){
                float color = (float)(image.imagePointer[i]) / 255.0;
                file << color << "\n";
            }
            
            file.close();
            printf("done!\n");
        }
        
        template <class AnyObject>
        static void image( brahand::ImageWrapper<AnyObject> image, std::string folder, std::string name){
            printf("Exporting %s \t.... ", name.c_str());
            
            
            auto minMax = image.imagePointer.minMax();
            printf("\t\t\t%f %f\n", (float)minMax.first, (float)minMax.second);
            auto size = image.size;
            
            std::string fullPath = folder + name + ".vtk";
            
            std::ofstream file; file.open( fullPath.c_str() );
            file << "# vtk DataFile Version 1.0\nUnstructured Grid Example\nASCII\nDATASET UNSTRUCTURED_GRID\n";
            
            file << "POINTS " << size.total() << " float\n";
            for(ushort z = 0 ; z < size.depth ; ++z){
                for(ushort y = 0 ; y < size.height ; ++y){
                    for(ushort x = 0 ; x < size.width ; ++x){
                        file << x << " " << size.height - y << " " << z << "\n";
                    }
                }
            }
            
            file << "CELLS " << size.total() << " " << size.total() * 2 << "\n";
            for(uint i = 0 ; i < size.total() ; ++i) file << "1 " << i << "\n";
            
            file << "CELL_TYPES " << size.total() << "\n";
            for(uint i = 0 ; i < size.total() ; ++i) file << "1 ";
            
            file << "\nCELL_DATA " << size.total() << "\nCOLOR_SCALARS float 1\n";
            for(uint i = 0 ; i < size.total() ; ++i){
                float color = (float)(image.imagePointer[i]) / minMax.second;
                file << color << "\n";
            }
            
            file.close();
            printf("done!\n");
        }
        
        template <class AnyObject>
        static void coordinates( brahand::ArrayPointer<AnyObject> array, brahand::ImageSize size, std::string folder, std::string name){
            printf("Exporting %s \t.... ", name.c_str());
            
            std::string fullpath = folder + name + ".vtk";
            std::ofstream file; file.open( fullpath.c_str() );
            
            file << "# vtk DataFile Version 1.0\nUnstructured Grid Example\nASCII\n\nDATASET UNSTRUCTURED_GRID\n";
            file << "POINTS " << array.count << " float\n";
            
            for(uint i = 0 ; i < array.count; ++i){
                auto coordinate = size(array[i]);
                file << coordinate.x << " " << size.height - coordinate.y << " " << coordinate.z << "\n";
            }
            
            file << "CELLS " << array.count << " " << array.count * 2 << "\n";
            for(uint i = 0 ;i < array.count ; ++i){ file << "1 " << i << "\n"; }
            
            file << "CELL_TYPES " << array.count << "\n";
            for(uint i = 0 ;i < array.count ; ++i){ file << "1 "; }
            
            file << "\nCELL_DATA " << array.count << "\nCOLOR_SCALARS lut 1\n";
            for(uint i = 0 ;i < array.count ; ++i){
                file <<  1.0 <<  "\n";
            }
            file.close();
            printf("done!\n");
        }

        template <class AnyObject>
        static void coordinates( brahand::ArrayPointer<AnyObject> array, brahand::ImageSize size, std::string folder, std::string name, brahand::ImageWrapper<float> density){
            printf("Exporting %s \t.... ", name.c_str());
//            auto minMax = density.imagePointer.minMax();
            
            float maxValue = std::numeric_limits<float>::min();
            for(brahand::uint i = 0 ; i < array.count; ++i){
                //auto c = size(array[i]);
                if(maxValue < density.imagePointer[array[i]]){
                    maxValue = density.imagePointer[array[i]];
                }
            }
            
            std::string fullpath = folder + name + ".vtk";
            std::ofstream file; file.open( fullpath.c_str() );
            
            file << "# vtk DataFile Version 1.0\nUnstructured Grid Example\nASCII\n\nDATASET UNSTRUCTURED_GRID\n";
            file << "POINTS " << array.count << " float\n";
            
            for(uint i = 0 ; i < array.count; ++i){
                auto coordinate = size(array[i]);
                file << coordinate.x << " " << size.height - coordinate.y << " " << coordinate.z << "\n";
            }
            
            file << "CELLS " << array.count << " " << array.count * 2 << "\n";
            for(uint i = 0 ;i < array.count ; ++i){ file << "1 " << i << "\n"; }
            
            file << "CELL_TYPES " << array.count << "\n";
            for(uint i = 0 ;i < array.count ; ++i){ file << "1 "; }
            
            file << "\nCELL_DATA " << array.count << "\nCOLOR_SCALARS lut 1\n";
            for(uint i = 0 ;i < array.count ; ++i){
//                file <<  1.0 <<  "\n";
                auto d = density.imagePointer[array[i]];
                if(d >= 0){
                    file << (float)d / maxValue <<  "\n";
                }
            }
            file.close();
            printf("done!\n");
        }

        
        template <class AnyObject>
        static void flux(brahand::ImageWrapper<AnyObject> density, brahand::ImageSize size, std::string folder, std::string name){
            auto minMax = density.imagePointer.minMax();
            assert(minMax.second != 0 );
            
            printf("Exporting %s \t.... ", name.c_str());
            
            std::string fullpath = folder + name + ".vtk";
            std::ofstream file; file.open( fullpath.c_str() );
            
            file << "# vtk DataFile Version 1.0\nUnstructured Grid Example\nASCII\n\nDATASET UNSTRUCTURED_GRID\n";
            
            brahand::uint total = 0;
            for(brahand::uint i = 0 ; i < density.size.total(); ++i){
                auto d = density.imagePointer[i];
                if(d >= 0){ ++total; }
            }

            file << "POINTS " << total << " float\n";
            for(brahand::uint i = 0 ; i < density.size.total(); ++i){
                auto d = density.imagePointer[i];
                if(d >= 0){
                    auto c = size(i);
                    file << c.x << " " << size.height - c.y << " " << c.z << "\n";
                }
            }
            file << "CELLS " << total << " " << total * 2 << "\n";
            for(uint i = 0 ;i < total ; ++i){ file << "1 " << i << "\n"; }

            file << "CELL_TYPES " << total << "\n";
            for(uint i = 0 ;i < total ; ++i){ file << "1 "; }

            file << "\nCELL_DATA " << total << "\nCOLOR_SCALARS lut 1\n";
            for(uint i = 0 ;i < density.size.total() ; ++i){
                auto d = density.imagePointer[i];
                if(d >= 0){
                    file << (float)d / (float)minMax.second <<  "\n";
                }
            }
            file.close();
            printf("done!\n");
        }

        template <class AnyObject>
        static void density(brahand::ImageWrapper<AnyObject> density, brahand::IndicesArray array, brahand::ImageSize size, std::string folder, std::string name){

            printf("Exporting %s \t.... ", name.c_str());
            
            std::string fullpath = folder + name + ".vtk";
            std::ofstream file; file.open( fullpath.c_str() );
            
            file << "# vtk DataFile Version 1.0\nUnstructured Grid Example\nASCII\n\nDATASET UNSTRUCTURED_GRID\n";
            
            file << "POINTS " << array.count << " float\n";
            float maxValue = std::numeric_limits<float>::min();
            for(brahand::uint i = 0 ; i < array.count; ++i){
                auto c = size(array[i]);
                file << c.x << " " << size.height - c.y << " " << c.z << "\n";
                if( maxValue < density.imagePointer[array[i]] ){
                    maxValue = density.imagePointer[array[i]];
                }
            }
            file << "CELLS " << array.count << " " << array.count * 2 << "\n";
            for(uint i = 0 ;i < array.count ; ++i){ file << "1 " << i << "\n"; }
            
            file << "CELL_TYPES " << array.count << "\n";
            for(uint i = 0 ;i < array.count ; ++i){ file << "1 "; }
            
            file << "\nCELL_DATA " << array.count << "\nCOLOR_SCALARS lut 1\n";
            for(uint i = 0 ;i < array.count ; ++i){
                auto d = density.imagePointer[array[i]];
                if(d >= 0){
                    file << (float)d / maxValue <<  "\n";
                }
            }
            file.close();
            printf("done!\n");
        }
    };

}



#endif /* Image_hpp */
