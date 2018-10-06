//
//  TrialEdges.h
//  B-Mesh
//
//  Created by Bryan Gonzales Vega on 9/11/16.
//  Copyright © 2016 Bryan Gonzales Vega. All rights reserved.
//

#ifndef TrialEdges_h
#define TrialEdges_h

#include "Trials.hpp"
#include "BMesh/Image.hpp"
#include "BMesh/Exporter.hpp"
#include "BMesh/Poisson2.hpp"
#include "BMesh/Triangulation.hpp"

void trialEdges(TrialPointer trial, std::string path) {
    
    double totalTime = 0.0;
    
#pragma mark - Image reading
    
    brahand::Image image;
    brahand::Profile<>::time("Opening " + trial->folderPath + trial->imageName, path, [&](){ image = brahand::Image(trial->folderPath + trial->imageName); });
    
#ifdef DEBUG_MODE
    brahand::VTKExport::grayImage(image, path, "image");
#endif
    
#pragma mark - Edges
    
    for(brahand::PixelType isovalue = 1 ; isovalue <= 254 ; ++isovalue){
        brahand::Image edgesImage;
        brahand::ImageIsovaluesVector params = {isovalue};
        totalTime += brahand::Profile<>::time("Edges", path, [&](){ edgesImage = image.edges(params); });
        brahand::IndicesArray edgeIndicesArray = edgesImage.features().first;
        brahand::VTKExport::coordinates(edgeIndicesArray, image.size, path, "edgeCoordinates" + std::to_string(isovalue));
    }
}

#endif /* TrialEdges_h */
