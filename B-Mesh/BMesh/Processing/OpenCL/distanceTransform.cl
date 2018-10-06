//#pragma OPENCL EXTENSION cl_khr_fp64 : enable // floating point support
//#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

/// http://canyouseethestars.com/can-you-see-the-stars-in-la/2014/6/2/opencl-and-all-zero-output-arrays
//
//kernel void naiveDT(global read_only unsigned int * edges,
//                    global write_only float * output)
//{
//    int x =  get_global_id(0);
//    int y =  get_global_id(1);
//    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1
//
//    unsigned short width   = get_global_size(0);
//    unsigned short height  = get_global_size(1);
//    unsigned short depth   = get_global_size(2);
//
//    double minDistance = 999999.0, distance;
//    short layer, row,col;
//    for(int i = 0 ; i < EDGES_COUNT-1 ; ++i){
//        unsigned int idx = edges[i];
//        layer = idx / (width * height);
//        idx -= (layer * width * height);
//        row = idx / width;
//        col = idx % width;
//
//        distance = ((x-col)*(x-col)) +  ((y-row)*(y-row)) + ((z-layer)*(z-layer));
//        if(distance < minDistance) {
//            minDistance = distance;
//        }
//    }
//
//    unsigned int index = (z * width  * height) + (y * width) + x;
//    output[index] = sqrt((float)minDistance);
//}

struct PropagationStatus{
    bool burned;
    float origin;
};

// This kernel prevents nvidia gpu errors.
kernel void init(   global read_only struct PropagationStatus * input,
                    global write_only struct PropagationStatus * output)
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);
    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1

    unsigned short width   = get_global_size(0);
    unsigned short height  = get_global_size(1);
    unsigned short depth   = get_global_size(2);

    unsigned int index = (z * width  * height) + (y * width) + x;
    output[index].burned = input[index].burned;
    output[index].origin = input[index].origin;
}

kernel void fastMarchingMethod(global read_only struct PropagationStatus * input,
                               global write_only struct PropagationStatus * output)
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);
    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1

    unsigned short width   = get_global_size(0);
    unsigned short height  = get_global_size(1);
    unsigned short depth   = get_global_size(2);

    unsigned int index = (z * width  * height) + (y * width) + x;
    //output[index].burned = input[index].burned;
    //output[index].origin = input[index].origin;


    float origin = input[index].origin;

    if(input[index].burned == true){
        // Copy into output
        output[index].burned = true;
        output[index].origin = origin;

        if(x+1 < width){
            unsigned int right = (z * width  * height) + (y * width) + (x + 1);
            if( input[right].burned == false ){
                output[right].burned = true;
                output[right].origin = origin;
            }
        }

        if(x-1 >= 0){
            unsigned int left = (z * width  * height) + (y * width) + (x-1);
            if(input[left].burned == false){
                output[left].burned = true;
                output[left].origin = origin;
            }
        }

        if(y+1 < height){
            unsigned int up = (z * width  * height) + ((y+1) * width) + (x);
            if(input[up].burned == false){
                output[up].burned = true;
                output[up].origin = origin;
            }
        }

        if(y-1 >= 0){
            unsigned int down = (z * width  * height) + ((y-1) * width) + (x);
            if(input[down].burned == false){
                output[down].burned = true;
                output[down].origin = origin;
            }
        }

        if(z+1 < depth){
            unsigned int far = ((z+1) * width  * height) + (y * width) + (x);
            if(input[far].burned == false){
                output[far].burned = true;
                output[far].origin = origin;
            }
        }

        if(z-1 >= 0){
            unsigned int near = ((z-1) * width  * height) + (y * width) + (x);
            if(input[near].burned == false){
                output[near].burned = true;
                output[near].origin = origin;
            }
        }
    }
}
