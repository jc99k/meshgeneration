//
//  TrialEdgeDensity.h
//  B-Mesh
//
//  Created by Bryan Gonzales Vega on 9/7/16.
//  Copyright © 2016 Bryan Gonzales Vega. All rights reserved.
//

#ifndef TrialEdgeDensity_h
#define TrialEdgeDensity_h

#include "Trials.hpp"
#include "BMesh/Image.hpp"
#include "BMesh/Exporter.hpp"
#include "BMesh/Poisson2.hpp"
#include "BMesh/Triangulation.hpp"


void edgeDensityTrial(TrialPointer trial, std::string path) {
    
    double totalTime = 0.0;
    
#pragma mark - Image reading
    
    brahand::Image image;
    brahand::Profile<>::time("Opening " + trial->folderPath + trial->imageName, path, [&](){ image = brahand::Image(trial->folderPath + trial->imageName); });
    
    brahand::VTKExport::grayImage(image, path, "image");
    printf("ImageSize: %d %d %d\n", image.size.width, image.size.height, image.size.depth);
    
#pragma mark - Edges
    
    brahand::Image edgesImage;
    totalTime += brahand::Profile<>::time("Edges", path, [&](){ edgesImage = image.edges(trial->isovalues); });
    
    brahand::IndicesArray edgeIndicesArray = edgesImage.features().first;
    brahand::VTKExport::coordinates(edgeIndicesArray, image.size, path, "edgeCoordinates");
    
    
#pragma mark - Gradient Flux Skeleton
    
    brahand::IndicesArray frameIndicesArray = edgesImage.addFrame();
    brahand::IndicesArray gammaIndicesArray = edgesImage.features().second;
    
    brahand::IndicesArray edgeWithFrameIndices(edgeIndicesArray, frameIndicesArray);
    brahand::VTKExport::coordinates(edgeWithFrameIndices, image.size, path, "edgeWithFrameCoordinates");
    
    brahand::Image skeletonImage; brahand::ImageWrapper<float> dt, fluxImage;
    totalTime += brahand::Profile<>::time("Skeleton",path, [&](){
        auto skel = edgesImage.jacobianSkeleton(dt, fluxImage, trial->blur, trial->flux);
        skeletonImage = skel.first;
    });
    brahand::IndicesArray skeletonIndicesArray = skeletonImage.features().first;
    
    brahand::VTKExport::coordinates(skeletonIndicesArray, image.size, path, "skeletonCoordinates");
    printf("\n>> DT min: %f, max: %f\n",dt.imagePointer.minMax().first, dt.imagePointer.minMax().second);
    printf("\n>> FLUX min: %f, max: %f\n",fluxImage.imagePointer.minMax().first, fluxImage.imagePointer.minMax().second);
    
    fluxImage = fluxImage.normalize<float>(0, 255);
    brahand::VTKExport::density(fluxImage, image.size, path, "fluxImage");
    
#pragma mark - Edge density
    
    brahand::ImageWrapper<float> density;
    totalTime += brahand::Profile<>::time("EdgeDensity",path, [&](){ density = image.edgeDensity(edgeWithFrameIndices,skeletonIndicesArray); });
    printf("\n>> EDGE_DENSITY min: %f, max: %f\n",density.imagePointer.minMax().first, density.imagePointer.minMax().second);
    brahand::VTKExport::density(density, edgeWithFrameIndices,  image.size, path, "edgeDensity");
    brahand::VTKExport::density(density, edgeIndicesArray,  image.size, path, "edgeDensityWithoutFrame");
    
#pragma mark - Propagation
    
    totalTime += brahand::Profile<>::time("Propagation", path,  [&](){ brahand::Image::propagation(density, dt, edgeWithFrameIndices, trial->waveLength); });
    brahand::ImageWrapper<float> normalizedDensity = density.normalize<float>(trial->minDensity, trial->maxDensity);
    
    printf("\n>> DENSITY_MAP min: %f, max: %f\n",density.imagePointer.minMax().first, density.imagePointer.minMax().second);
    printf("\n>> DENSITY_MAP_NORM min: %f, max: %f\n",normalizedDensity.imagePointer.minMax().first, normalizedDensity.imagePointer.minMax().second);
    
    if( dt.size.depth == 1 ){
        dt.save2DElevationMap(path, "elevationMap");
    }
    
    brahand::VTKExport::image<float>(dt, path, "dt");
    brahand::VTKExport::image(density, path, "densityMap");
    
#pragma mark - Poisson sampling (edges)
    
    brahand::Poisson sampler( image.size, normalizedDensity );
    brahand::IndicesArray edgeSamples, frameSamples, samples;
    
    totalTime += brahand::Profile<>::time("Edge Samples", path, [&](){ edgeSamples = sampler.sampling( edgeIndicesArray ); });
    frameSamples = sampler.sampling( frameIndicesArray );
    
    ////#pragma mark - Maximal Poisson Sampling (edges)
    ////    edgeSamples = sampler.maximal( edgeIndicesArray, edgeSamples );
    
#pragma mark - Poisson sampling (interior)
    
    totalTime += brahand::Profile<>::time("Samples",path, [&](){ samples = sampler.sampling( gammaIndicesArray ); });
    
    ////#pragma mark - Maximal Poisson Sampling (interior)
    ////    samples = sampler.maximal( gammaIndicesArray, samples );
    ////    samples = brahand::IndicesArray(samples, edgeSamples);
    
    printf("Total samples: %d\n", samples.count);
    brahand::VTKExport::coordinates(edgeSamples, image.size, path, "edgeSamples", normalizedDensity);
    brahand::VTKExport::coordinates(frameSamples, image.size, path, "frameSamples", normalizedDensity);
    brahand::VTKExport::coordinates(samples, image.size, path, "interiorSamples", normalizedDensity);
    
#pragma mark - Triangulation
    
    Histogram interiorHistogram, edgeHistogram;
    std::shared_ptr<CGALTriangulation> interiorTriangulation, edgeTriangulation;
    
    totalTime += brahand::Profile<>::time("Triangulation",path,  [&](){
        if(image.size.depth == 1){
            interiorTriangulation = std::make_shared<CGALTriangulation2D>( samples, image.size);
            edgeTriangulation = std::make_shared<CGALTriangulation2D>( frameSamples, image.size);
        } else {
            interiorTriangulation = std::make_shared<CGALTriangulation3D>( samples, image.size);
            edgeTriangulation = std::make_shared<CGALTriangulation3D>( frameSamples, image.size);
        }
    });
    
    interiorHistogram = interiorTriangulation->generate(path, "interiorTriangulation", image);
    edgeHistogram = edgeTriangulation->generate(path, "edgeTriangulation", image);
    
    saveHistogram(path, "histogram.txt", edgeHistogram, interiorHistogram);
    printf(">> Total time: %.3f sec\n", totalTime);
}


#endif /* TrialEdgeDensity_h */
