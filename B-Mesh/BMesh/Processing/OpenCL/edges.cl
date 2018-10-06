#pragma OPENCL EXTENSION cl_khr_fp64 : enable // floating point support
//#pragma OPENCL EXTENSION cl_intel_printf : enable
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable


kernel void edges(global read_only short * input,
                  global read_only short * isovalues,
                  global write_only short * output)
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);
    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1

    unsigned short width   = get_global_size(0);
    unsigned short height  = get_global_size(1);
    unsigned short depth   = get_global_size(2);

    unsigned int index = (z * width  * height) + (y * width) + x, dx,dy,dz, isoIndex;
    short current = input[index], neighbor, min, max;
    unsigned short a,b,c;

    unsigned short cinit = (depth == 1)? 0 : z-1,   climit = (depth == 1)? 0 : z+1;
    unsigned short zinit = (depth == 1)? 0 : 1,     zlimit = (depth == 1)? depth : depth-1;

    // This line prevent nvidia gpu errors.
    output[(z * width  * height) + (y * width) + x] = 0;

    if( x < width - 1 && y < height - 1 && x > 0 && y > 0 && z >= zinit && z < zlimit){
        for ( c = cinit; c <= climit; c++ ){
            for ( a = y-1; a <= y+1; a++ ){
                for ( b = x-1; b <= x+1; b++ ){
                    if ( a == b && b==c ) continue;
                    neighbor = input[(c * width  * height) + (a * width) + b];

                    min = (current <= neighbor)? current : neighbor;
                    max = (current <= neighbor)? neighbor : current;

                    for(isoIndex = 0 ; isoIndex < ISOVALUES_COUNT ; ++isoIndex){
                        if ( min <= isovalues[isoIndex] && isovalues[isoIndex] <= max ) {
                            dy = (y+a)/2.0;	dx = (x+b)/2.0; dz = (z+c)/2.0;
                            output[(dz * width  * height) + (dy * width) + dx] = 255;
                        }
                    }
                }
            }
        }
    }
}
