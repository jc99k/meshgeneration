#ifndef MESHGENERATION_H
#define MESHGENERATION_H

#include "Trials.hpp"
#include "BMesh/Image.hpp"
#include "BMesh/Exporter.hpp"
#include "BMesh/Poisson2.hpp"
#include "BMesh/Triangulation.hpp"


void meshGeneration(TrialPointer trial, std::string path) {

    double totalTime = 0.0;
    brahand::OutputFileAndConsole output(path + "info.txt");

#pragma mark - Image reading

    brahand::Image image;
    brahand::Profile<>::time("Opening " + trial->folderPath + trial->imageName, path, [&](){ image = brahand::Image(trial->folderPath + trial->imageName); });
    
#ifdef DEBUG_MODE
    brahand::VTKExport::grayImage(image, path, "image");
#endif
    output << "ImageSize\t" << image.size.width << " " << image.size.height << " " << image.size.depth << "\n";

#pragma mark - Edges

//    for (uint i = 0 ;)
    
    brahand::Image edgesImage;
    totalTime += brahand::Profile<>::time("Edges", path, [&](){ edgesImage = image.edges(trial->isovalues); });

    brahand::IndicesArray edgeIndicesArray = edgesImage.features().first;
    brahand::VTKExport::coordinates(edgeIndicesArray, image.size, path, "edgeCoordinates");
    
    output << "EdgeCount\t" << edgeIndicesArray.count << "\n";

#pragma mark - Gradient Flux Skeleton
    
    brahand::IndicesArray frameIndicesArray = edgesImage.addFrame();
    brahand::IndicesArray interiorIndicesArray = edgesImage.features().second;
    
    output << "FrameCount\t" << frameIndicesArray.count << "\n";
    output << "InteriorCount\t" << interiorIndicesArray.count << "\n";
    
    brahand::IndicesArray edgeWithFrameIndices(edgeIndicesArray, frameIndicesArray);
    brahand::VTKExport::coordinates(edgeWithFrameIndices, image.size, path, "edgeWithFrameCoordinates");

    brahand::Image skeletonImage; brahand::ImageWrapper<float> dt, fluxImage;
    totalTime += brahand::Profile<>::time("Skeleton",path, [&](){
        auto skel = edgesImage.jacobianSkeleton(dt, fluxImage, 0.0, trial->flux);
        skeletonImage = skel.first;
    });
    
#ifdef DEBUG_MODE
    brahand::VTKExport::image(skeletonImage, path, "skeletonImage");
#endif
    
    brahand::IndicesArray skeletonIndicesArray = skeletonImage.features().first;

    brahand::VTKExport::coordinates(skeletonIndicesArray, image.size, path, "skeletonCoordinates");
    output << "SkeletonCount\t" << skeletonIndicesArray.count << "\n";
    output << "Dt\t" << dt.imagePointer.minMax().first << " " << dt.imagePointer.minMax().second << "\n";
    output << "Flux\t" << fluxImage.imagePointer.minMax().first << " " << fluxImage.imagePointer.minMax().second << "\n";
    
#ifdef DEBUG_MODE
    fluxImage = fluxImage.normalize<float>(0, 255);
    brahand::VTKExport::flux(fluxImage, image.size, path, "fluxImage");
#endif
    
#pragma mark - Edge density
    
    brahand::ImageWrapper<float> density;
    totalTime += brahand::Profile<>::time("EdgeDensity",path, [&](){ density = image.edgeDensity(edgeWithFrameIndices,skeletonIndicesArray); });
    output << "EdgeDensity\t" << density.imagePointer.minMax().first <<" "<< density.imagePointer.minMax().second << "\n";

    brahand::ImageWrapper<float> edgeDensityWithoutFrame = density;

#pragma mark - Propagation

    float waveLenght;
    totalTime += brahand::Profile<>::time("Propagation", path,  [&](){ waveLenght = brahand::Image::propagation(density, dt, edgeWithFrameIndices, trial->waveLength); });

    
    brahand::VTKExport::image(density, path, "densityMap");
    brahand::VTKExport::density(density, edgeWithFrameIndices,  image.size, path, "edgeDensity");
    brahand::VTKExport::density(density, edgeIndicesArray,  image.size, path, "edgeDensityWithoutFrame");
    
    brahand::ImageWrapper<float> normalizedDensity = density.normalize<float>(trial->minDensity, trial->maxDensity);
    output << "WaveLenght\t" << waveLenght << "\n";
    output << "DensityMap\t" << density.imagePointer.minMax().first << " " << density.imagePointer.minMax().second << "\n";
    output << "DensityMap(n)\t" << normalizedDensity.imagePointer.minMax().first << " " << normalizedDensity.imagePointer.minMax().second << "\n";


#ifdef DEBUG_MODE
    if( dt.size.depth == 1 ){ dt.save2DElevationMap(path, "elevationMap"); }
    brahand::VTKExport::image<float>(dt, path, "dt");
    brahand::VTKExport::image(normalizedDensity, path, "normalizedDensityMap");
#endif

#pragma mark - Poisson sampling (edges)

    brahand::Poisson sampler( image.size, normalizedDensity );
    brahand::IndicesArray edgeSamples, frameSamples, samples;
    
    totalTime += brahand::Profile<>::time("Edge Samples", path, [&](){ edgeSamples = sampler.sampling( edgeIndicesArray ); });
    output << "EdgeSamplesCount\t" << edgeSamples.count << "\n";
    frameSamples = sampler.sampling( frameIndicesArray );
    
#pragma mark - Poisson sampling (interior)
    
    totalTime += brahand::Profile<>::time("Samples",path, [&](){ samples = sampler.sampling( interiorIndicesArray ); });
    output << "TotalSamplesCount\t" << samples.count << "\n";
    
    brahand::VTKExport::coordinates(edgeSamples, image.size, path, "edgeSamples", density);
    brahand::VTKExport::coordinates(frameSamples, image.size, path, "frameSamples", density);
    brahand::VTKExport::coordinates(samples, image.size, path, "interiorSamples", density);

#pragma mark - Triangulation

    Histogram interiorHistogram, edgeHistogram;
    
    std::shared_ptr<CGALTriangulation> interiorTriangulation, edgeTriangulation;

    if(image.size.depth == 1) {
        interiorTriangulation = std::make_shared<CGALTriangulation2D>( samples, image.size);
        edgeTriangulation = std::make_shared<CGALTriangulation2D>( frameSamples, image.size);
        
        totalTime += brahand::Profile<>::time("InteriorTriangulation",path,  [&](){ interiorTriangulation->triangulate(); });
        totalTime += brahand::Profile<>::time("EdgeTriangulation",path,  [&](){ edgeTriangulation->triangulate(); });
    } else {
    }
    
    output << "EdgeTriangulationCount\t" << edgeTriangulation->elementsCount << "\n";
    output << "InteriorTriangulationCount\t" << interiorTriangulation->elementsCount << "\n";
    
    edgeHistogram = edgeTriangulation->generateHistogram(path, "edgeTriangulation", image);
    interiorHistogram = interiorTriangulation->generateHistogram(path, "interiorTriangulation", image);
    
    saveHistogram(path, "histogram.txt", edgeHistogram, interiorHistogram);
    output << "Total time\t" << totalTime << "\n";
}


#endif // MESHGENERATION_H
