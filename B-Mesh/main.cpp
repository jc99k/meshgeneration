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
#include <string>
#include "MeshGeneration.h"
#include "Yaml.hpp"

int main(int argc, char** argv) {
    std::setbuf(stdout, NULL);
    // std::vector<TrialPointer> experiments {
    //     BUILD_TRIAL(Dragonfruit),
    // };

    // for( auto trial : experiments ){
    //     const std::string path = RESULTS_PATH + trial->identifier + "/";
    //     mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );

    //     meshGeneration(trial, path);
    // }

    TrialPointer trial = BUILD_TRIAL(CustomTrial);

    /* Parse YAML */
    std::string case_filename = argv[1];
    std::string case_name = argv[2];

    Yaml::Node root;
    Yaml::Parse(root, case_filename.c_str());
	Yaml::Node& selected = root["cases"][case_name];

    trial->identifier = selected["identifier"].As<std::string>();
    trial->folderPath = ASSETS_PATH;
    trial->imageName = selected["imageName"].As<std::string>();
    trial->waveLength = selected["waveLength"].As<int>();
    trial->flux = selected["flux"].As<float>();
    trial->minDensity = selected["minDensity"].As<float>();
    trial->maxDensity = selected["maxDensity"].As<float>();
    for(auto it = selected["isovalues"].Begin(); it != selected["isovalues"].End(); it++) trial->isovalues.push_back((*it).second.As<float>());    

    const std::string path = RESULTS_PATH + trial->identifier + "/";
    mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );

    meshGeneration(trial, path);

    return 0;
}
