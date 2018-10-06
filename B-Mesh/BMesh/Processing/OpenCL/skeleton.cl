#pragma OPENCL EXTENSION cl_khr_fp64 : enable // floating point support
#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

struct GradientElement{
    float x; float y; float z;
};


kernel void flux(global read_only short * image,
                 global read_only struct GradientElement * gradient,
                 global write_only float * output)
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);
    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1

    unsigned short width   = get_global_size(0);
    unsigned short height  = get_global_size(1);
    unsigned short depth   = get_global_size(2);

    unsigned int index = (z * width  * height) + (y * width) + x;

    // This line prevent nvidia gpu errors
    output[index] = 0;

    if ( image[index] == 0 ){
        int stop = 0;
        float f = 0.0;
        int count = 0;

        for (int k = -1; k<=1; ++k){
            for (int l = -1; l<= 1; ++l){
                for (int m = -1; m<= 1; ++m) {
                        if (stop==1) continue;

                        if ((x + k<0) || (x + k>=width) || (y + l<0) || (y + l>=height) ||
                            (z + m<0) || (z + m>=depth) || (k==0 && l==0 && m==0)) continue;
                                    ++count;

                        unsigned int nIndex = ((z+m) * width  * height) + ((y+l) * width) + (x+k);

                        if (image[nIndex]==255) { stop = 1; continue; }

                        float gx = gradient[nIndex].x;
                        float gy = gradient[nIndex].y;
                        float gz = gradient[nIndex].z;

                        f+=(float)(gx*(float)k + gy*(float)l/1.0 + gz*(float)m/1.0) / sqrt((float)(k*k + l*l + m*m));
                }
            }
        }

        if (stop==1 || count==0) output[index] = 0;
        else output[index] = f/(float) count;
    }
}

/// http://canyouseethestars.com/can-you-see-the-stars-in-la/2014/6/2/opencl-and-all-zero-output-arrays

kernel void skeletonByLocalMaxima(global read_only float * input, // Distances array
                                  global write_only short * output) // Skeleton Image
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);
    int z =  get_global_id(2); // in 2D z = 0 not 1, but depth = 1

    unsigned short width   = get_global_size(0);
    unsigned short height  = get_global_size(1);
    unsigned short depth   = get_global_size(2);

    unsigned int index = (z * width  * height) + (y * width) + x;

    float maximum = -9999.0;
    if(x+1 < width){
        unsigned int right = (z * width  * height) + (y * width) + (x+1);
        if(input[right] > maximum){ maximum = input[right]; }
    }

    if(x-1 >= 0){
        unsigned int left = (z * width  * height) + (y * width) + (x-1);
        if(input[left] > maximum){ maximum = input[left]; }
    }

    if(y+1 < height){
        unsigned int up = (z * width  * height) + ((y+1) * width) + (x);
        if(input[up] > maximum){ maximum = input[up]; }
    }

    if(y-1 >= 0){
        unsigned int down = (z * width  * height) + ((y-1) * width) + (x);
        if(input[down] > maximum){ maximum = input[down]; }
    }

    if(z+1 < depth){
        unsigned int far = ((z+1) * width  * height) + (y * width) + (x);
        if(input[far] > maximum){ maximum = input[far]; }
    }

    if(z-1 >= 0){
        unsigned int near = ((z-1) * width  * height) + (y * width) + (x);
        if(input[near] > maximum){ maximum = input[near]; }
    }

    if(input[index] >= maximum){
        output[index] = 255;
    }
}


__kernel void gaussianBlur(global read_only float * input,
                           constant float * mask,
                           global float * blurredImage,
                           private int maskSizeUnused )
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);

    unsigned short width    = get_global_size(0);
    unsigned short height   = get_global_size(1);
    unsigned int index      = (y * width) + x;

    float maskSize = 3.0/ 2.0;
    if(x >= floor(maskSize) && x < width - floor(maskSize) &&
       y >= floor(maskSize) && y < height - floor(maskSize) )
    {
        float sum = 0.0f;
        for (int m = 0 ; m < 3 ; ++m) {
            for (int n = 0 ; n < 3 ; ++n){
                float maskValue = mask[(m * 3) + n];
                unsigned int idx = x - (3/2) + m;
                unsigned int idy = y - (3/2) + n;
                float value = input[(idy * width) + idx];
                sum += value * maskValue;
            }
        }
        blurredImage[index] = sum;
    }
}


__kernel void gradient(global read_only float * input,
                       constant float * mask,
                       global float * blurredImage)
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);

    unsigned short width    = get_global_size(0);
    unsigned short height   = get_global_size(1);
    unsigned int index      = (y * width) + x;

    float maskSize = 3.0/ 2.0;
    if(x >= floor(maskSize) && x < width - floor(maskSize) &&
       y >= floor(maskSize) && y < height - floor(maskSize) )
    {
        float sum = 0.0f;
        for (int m = 0 ; m < 3 ; ++m) {
            for (int n = 0 ; n < 3 ; ++n){
                float maskValue = mask[(m * 3) + n];
                unsigned int idx = x - (3/2) + m;
                unsigned int idy = y - (3/2) + n;
                float value = input[(idy * width) + idx];
                sum += value * maskValue;
            }
        }
        blurredImage[index] = sum;
    }
}


__kernel void gradientMagnitude(global read_only float * gradRows,
                                global read_only float * gradCols,
                                global float * magnitude)
{
    int x =  get_global_id(0);
    int y =  get_global_id(1);

    unsigned short width    = get_global_size(0);
    unsigned short height   = get_global_size(1);
    unsigned int index      = (y * width) + x;

    magnitude[index] = sqrt((gradRows[index]*gradRows[index]) + (gradCols[index] * gradCols[index]));
}
