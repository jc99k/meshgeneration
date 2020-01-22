//
//  main.cpp
//  B-Mesh
//
//  Created by Bryan Gonzales Vega on 6/14/16.
//  Copyright © 2016 Bryan Gonzales Vega. All rights reserved.
//

//#define PROCESSING_TYPE OpenCL
#define DEBUG_MODE

#if defined(__APPLE__) || defined(__MACOSX)
    #define IMAGEMAGICK_PATH "/usr/local/bin/convert"
    #define RESULTS_PATH "/Users/rgonzales/Public/"
    #define PROJECT_PATH ""
    #define ASSETS_PATH PROJECT_PATH "Assets/"
    #define KERNELS_PATH PROJECT_PATH ""
#else
    #define IMAGEMAGICK_PATH "/usr/bin/convert"
    #define PROJECT_PATH "./"
    #define RESULTS_PATH PROJECT_PATH "Results/"
    #define ASSETS_PATH PROJECT_PATH "Assets/"
    #define KERNELS_PATH PROJECT_PATH "B-Mesh/BMesh/Processing/OpenCL/"
#endif

#include <sys/stat.h>
#include "MeshGeneration.h"

int main() {
    std::setbuf(stdout, NULL);
    std::vector<TrialPointer> experiments {
        BUILD_TRIAL(Dragon),
    };

    for( auto trial : experiments ){
        const std::string path = RESULTS_PATH + trial->identifier + "/";
        mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );

        meshGeneration(trial, path);
    }

    return 0;
}
