#pragma OPENCL EXTENSION cl_khr_fp64 : enable // floating point support
//#pragma OPENCL EXTENSION cl_intel_printf : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable


//kernel void edges(global read_only short * input,
//                  global read_only short * isovalues,
//                  global write_only short * output)
//{
//    int x =  get_global_id(0);
//    int y =  get_global_id(1);
//    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1
//    
//    unsigned short width   = get_global_size(0);
//    unsigned short height  = get_global_size(1);
//    unsigned short depth   = get_global_size(2);
//    
//    unsigned int index = (z * width  * height) + (y * width) + x, dx,dy,dz, isoIndex;
//    short current = input[index], neighbor, min, max;
//    unsigned short a,b,c;
//    
//    unsigned short cinit = (depth == 1)? 0 : z-1,   climit = (depth == 1)? 0 : z+1;
//    unsigned short zinit = (depth == 1)? 0 : 1,     zlimit = (depth == 1)? depth : depth-1;
//    
//    if( x < width - 1 && y < height - 1 && x > 0 && y > 0 && z >= zinit && z < zlimit){
//        for ( c = cinit; c <= climit; c++ ){
//            for ( a = y-1; a <= y+1; a++ ){
//                for ( b = x-1; b <= x+1; b++ ){
//                    if ( a == b && b==c ) continue;
//                    neighbor = input[(c * width  * height) + (a * width) + b];
//                    
//                    min = (current <= neighbor)? current : neighbor;
//                    max = (current <= neighbor)? neighbor : current;
//                    
//                    for(isoIndex = 0 ; isoIndex < ISOVALUES_COUNT ; ++isoIndex){
//                        if ( min <= isovalues[isoIndex] && isovalues[isoIndex] <= max ) {
//                            dy = (y+a)/2.0;	dx = (x+b)/2.0; dz = (z+c)/2.0;
//                            output[(dz * width  * height) + (dy * width) + dx] = 255;
//                        }
//                    }
//                }
//            }
//        }
//    }
//}

kernel void naiveDT(global read_only unsigned int * edges,
                  global write_only float * output)
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);
    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1
    
    unsigned short width   = get_global_size(0);
    unsigned short height  = get_global_size(1);
    unsigned short depth   = get_global_size(2);
    
    unsigned int index = (z * width  * height) + (y * width) + x, dx,dy,dz, isoIndex;
    
    output[index] = 32.3;
}

//kernel void naiveDistanceTransform(global read_only unsigned int * edges,
//                                   const unsigned int edgesCount,
//                                   global write_only float * output)
//{
//    int x =  get_global_id(0);
//    int y =  get_global_id(1);
//    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1
//    
//    unsigned short width    = get_global_size(0);
//    unsigned short height   = get_global_size(1);
//    unsigned short depth    = get_global_size(2);
//    unsigned int index      = (z * width * height) + (y * width) + x, idx;
//
//    
//
//    float minDistance = 99999.0;
//    for(int i = 0 ; i < edgesCount; ++i){
//        idx = edges[i];
//
//        short layer = idx / (width * height);
//        idx -= (layer * width * height);
//        short row = idx / width;
//        short col = idx % width;
//
//        float distance = ((x-col)*(x-col)) +  ((y-row)*(y-row)) + ((z-layer)*(z-layer));
//        
//        if(distance < minDistance) minDistance = sqrt(distance);
//    }
//
//    output[index] = 123.02;
////    short nx, ny, nz;
////
////    short mask[6][3] = {
////        {-1,0,0}, {0,-1,0}, {0,1,0}, {1,0,0}, // 2D
////        {0,0,-1}, {0,0,1} // 3D
////    };
////
////
////    if(wave[index].burned == true){
////
////        for(short offset = 0 ; offset < ((depth == 1)?4:6) ; ++offset){
////            nx = x + mask[offset][0];
////            ny = y + mask[offset][1];
////            nz = z + mask[offset][2];;
////
////            if( nx >= 0 && nx < width && ny >= 0 && ny < height && nz >= 0 && nz < depth){
////
////                nindex = (nz * width  * height) + (ny * width) + nx;
////                if( wave[nindex].burned == false){
////                    output[nindex].density = time;
////                    output[nindex].burned = true;
////
////                    *frontSize = 1;
////                }
////            }
////        }
////    }
//}




//struct TracePropagation {
//    short density;
//    bool burned;
//    unsigned int origin;
//    short time;
//};
//
//kernel void fastMarchingMethod(global read_only struct TracePropagation * wave,
//                               const short time,
//                               global read_only struct TracePropagation * output,
//                               global write_only unsigned short * frontSize)
//{
//    int x =  get_global_id(0);
//    int y =  get_global_id(1);
//    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1
//
//    unsigned short width   = get_global_size(0);
//    unsigned short height  = get_global_size(1);
//    unsigned short depth   = get_global_size(2);
//    unsigned int index = (z * width  * height) + (y * width) + x, nindex;
//    
//    short nx, ny, nz;
//    
//    short mask[6][3] = {
//        {-1,0,0}, {0,-1,0}, {0,1,0}, {1,0,0}, // 2D
//        {0,0,-1}, {0,0,1} // 3D
//    };
//    
//
//    if(wave[index].burned == true){
//    
//        for(short offset = 0 ; offset < ((depth == 1)?4:6) ; ++offset){
//            nx = x + mask[offset][0];
//            ny = y + mask[offset][1];
//            nz = z + mask[offset][2];;
//            
//            if( nx >= 0 && nx < width && ny >= 0 && ny < height && nz >= 0 && nz < depth){
//                
//                nindex = (nz * width  * height) + (ny * width) + nx;
//                if( wave[nindex].burned == false){
//                    output[nindex].density = time;
//                    output[nindex].burned = true;
//
//                    *frontSize = 1;
//                }
//            }
//        }
//    }
//}

