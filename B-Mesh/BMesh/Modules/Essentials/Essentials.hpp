#ifndef Essentials_hpp
#define Essentials_hpp

#define OpenMP 0
#define OpenCL 1

//#ifndef PROCESSING_TYPE
//#error Macro PROCESSING_TYPE is required!
//#endif

#include <memory>
#include <assert.h>
#include <string>
#include <chrono>
#include <limits>
#include <vector>
#include <iostream>
#include <fstream>

//#if defined(__APPLE__) || defined(__MACOSX)
//    #include <libiomp/omp.h>
//#else
//    #include <omp.h>
//#endif

namespace brahand {

    /// [0 - 4,294,967,296>, 4 bytes
    typedef unsigned int uint;

    /// [0 - 65536>, 2 bytes
    typedef unsigned short ushort; // uchar requires cast with numerical operations.

    // Array with shared_ptr
    template <class AnyObject>
    struct ArrayPointer {
        uint count;
        std::shared_ptr<AnyObject> content;

        ArrayPointer() = default;
        ArrayPointer(uint size){
            count = size;
            content = std::shared_ptr<AnyObject>(new AnyObject[count], [](AnyObject * object){
                delete[] object;
            });
        }
        ArrayPointer(uint size, AnyObject defaultValue) : ArrayPointer(size){
            auto pointer = content.get();
#pragma omp parallel for
            for(uint i = 0 ; i < count ; ++i){
                *(pointer + i) = defaultValue;
            }
        }
        ArrayPointer(ArrayPointer<AnyObject> a, ArrayPointer<AnyObject> b) : ArrayPointer(a.count + b.count){
            auto pointer = content.get();
            uint idx = 0;
            for(uint i = 0 ; i < a.count ; ++i){
                *(pointer + idx) = a[i];
                ++idx;
            }
            for(uint i = 0 ; i < b.count ; ++i){
                *(pointer + idx) = b[i];
                ++idx;
            }
        }
        ArrayPointer(uint size, std::shared_ptr<AnyObject> newContent){
            count = size;
            content = newContent;
        }
        ArrayPointer(std::vector<AnyObject> vector) : ArrayPointer((uint)vector.size()) {
            auto pointer = content.get();
#pragma omp parallel for
            for(uint i = 0 ; i < vector.size() ; ++i){
                *(pointer + i) = vector[i];
            }
        }

        inline AnyObject& operator[] (uint index) const{
            return *(content.get() + index);
        }

        inline std::pair<AnyObject, AnyObject> minMax() const {
            AnyObject minimum = std::numeric_limits<AnyObject>::max();
            AnyObject maximum = std::numeric_limits<AnyObject>::min();

#pragma omp parallel for reduction(min:minimum) reduction(max:maximum)
            for(uint i = 0 ; i < count ; ++i){
                const AnyObject value = *(content.get() + i);
                if( value < minimum ){ minimum = value; }
                if( value > maximum ){ maximum = value; }
            }
            return {minimum, maximum};
        }

        inline void print(){
            printf("Array with %u elements: ", count);
            for( uint i = 0 ; i < count ; ++i){ std::cout << this->operator[](i) << " "; }
            printf("\n");
        }
    };

    /// Array of [0 - 4,294,967,296>, 4 bytes
    typedef ArrayPointer<uint> IndicesArray;
    
    template <class AnyObject>
    struct CoordinateWrapper{
        AnyObject x,y,z;
    };


    // Profiling time
    template <typename TimeObject = std::chrono::milliseconds>
    struct Profile {

        template <typename Function, typename ...Arguments>
        static double time(std::string process, std::string path, Function && func, Arguments &&... args){
            std::string name = "linear";
            
//#if PROCESSING_TYPE == OpenMP 
//            name = "openmp";
//#elif PROCESSING_TYPE == OpenCL
//            name = "opencl";
//#endif
            
            std::ofstream os;
            os.open(path + name + ".txt", std::ofstream::out | std::ofstream::app);
            
            printf("%s \t", process.c_str());
            os << process << ",";

            auto start  = std::chrono::steady_clock::now();
            std::forward<decltype(func)>(func)(std::forward<Arguments>(args)...);
            auto end    = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<TimeObject>(end-start).count();

            printf("%.3f\n", duration * 0.001 );
            os << (double)duration * 0.001 << "\n";
            os.close();
            return (double)duration * 0.001;
        }
    };
    
    struct OutputFileAndConsole : std::ofstream {
        OutputFileAndConsole(const std::string& fileName) : std::ofstream(fileName, std::ofstream::out | std::ofstream::app)
        , fileName(fileName){};
        
        const std::string fileName;
    };
    
    template <typename T>
    OutputFileAndConsole& operator<<(OutputFileAndConsole& strm, const T& var) {
        std::cout << var;
        static_cast<std::ofstream&>(strm) << var;
        return strm;
    };

}

#endif /* Essentials_hpp */
