//
//  SkeletonTrials.h
//  B-Mesh
//
//  Created by Bryan Gonzales Vega on 9/5/16.
//  Copyright © 2016 Bryan Gonzales Vega. All rights reserved.
//

#ifndef SkeletonTrials_h
#define SkeletonTrials_h

#include "Trials.hpp"
#include "BMesh/Image.hpp"
#include "BMesh/Exporter.hpp"
#include "BMesh/Poisson2.hpp"
#include "BMesh/Triangulation.hpp"


void skeletonTrial(TrialPointer trial, std::string path) {
    
    double totalTime = 0.0;
    
#pragma mark - Image reading
    
    brahand::Image image;
    brahand::Profile<>::time("Opening " + trial->folderPath + trial->imageName, path, [&](){ image = brahand::Image(trial->folderPath + trial->imageName); });
    brahand::VTKExport::grayImage(image, path, "image");
    
#pragma mark - Edges
    
    brahand::Image edgesImage;
    totalTime += brahand::Profile<>::time("Edges", path, [&](){ edgesImage = image.edges(trial->isovalues); });
    
    brahand::IndicesArray edgeIndicesArray = edgesImage.features().first;
    brahand::VTKExport::coordinates(edgeIndicesArray, image.size, path, "edgeCoordinates");
    
    
#pragma mark - Gradient Flux Skeleton
    float minFlux = 0.0;
    brahand::IndicesArray frameIndicesArray = edgesImage.addFrame();
    brahand::IndicesArray gammaIndicesArray = edgesImage.features().second;
    
    brahand::IndicesArray edgeWithFrameIndices(edgeIndicesArray, frameIndicesArray);
    brahand::VTKExport::coordinates(edgeWithFrameIndices, image.size, path, "edgeWithFrameCoordinates");
    
    brahand::ImageWrapper<float> unuseddt, unusedflux;
    minFlux = edgesImage.jacobianSkeleton(unuseddt, unusedflux, trial->blur, trial->flux).second;
    printf("Min Flux: %f\n", minFlux);
    
    std::ofstream fluxos;
    fluxos.open(path + "flux.txt", std::ofstream::out | std::ofstream::app);
    
    float ratio = 0.05;
    int trialIndex = 0;
    for(float flux = minFlux ; flux < 0.0 ; flux+=ratio, trialIndex++){
        fluxos << trialIndex << "," << flux << "\n";
        brahand::Image skeletonImage; brahand::ImageWrapper<float> dt, fluxImage;
        auto skel = edgesImage.jacobianSkeleton(dt, fluxImage, trial->blur, flux);
        skeletonImage = skel.first;
    
        brahand::IndicesArray skeletonIndicesArray = skeletonImage.features().first;
        
        brahand::VTKExport::coordinates(skeletonIndicesArray, image.size, path, "skeletonCoordinates" + std::to_string(trialIndex));
        printf("\nDT min: %f, max: %f\n",dt.imagePointer.minMax().first, dt.imagePointer.minMax().second);
        
#pragma mark - Edge density
        
        brahand::ImageWrapper<float> density;
        totalTime += brahand::Profile<>::time("EdgeDensity" + std::to_string(trialIndex),path, [&](){
            density = image.edgeDensity(edgeWithFrameIndices,skeletonIndicesArray);
        });
        
        brahand::VTKExport::density(density,edgeIndicesArray,image.size, path, "edgeDensity" + std::to_string(trialIndex));
        
        brahand::Image::propagation(density, dt, edgeWithFrameIndices, trial->waveLength);
        brahand::ImageWrapper<float> normalizedDensity = density.normalize<float>(trial->minDensity, trial->maxDensity);
        
        
        brahand::IndicesArray edgeSamples, frameSamples, samples;
        brahand::Poisson sampler( image.size, normalizedDensity );
        edgeSamples = sampler.sampling( edgeIndicesArray );
        
        brahand::VTKExport::coordinates(edgeSamples, image.size, path, "edgeSamples" + std::to_string(trialIndex), normalizedDensity);
    }
    
    fluxos.close();
}


#endif /* SkeletonTrials_h */
