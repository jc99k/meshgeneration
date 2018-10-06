#ifndef TRIALDENSITY_H
#define TRIALDENSITY_H


#include "Trials.hpp"
#include "BMesh/Image.hpp"
#include "BMesh/Exporter.hpp"
#include "BMesh/Poisson2.hpp"
#include "BMesh/Triangulation.hpp"

#include <fstream>
#include <sstream>
#include <map>
#include <numeric>
#include <algorithm>

struct Resume{
    uint id;
    float minDensity;
    float maxDensity;
    uint samples;
    std::vector<uint> histogram;
    std::pair<float, float> minRange;
    std::pair<float, float> maxRange;
};

std::map<float,std::map<float, std::vector<unsigned int>>> histogramAnalysis;
std::map<float,std::map<float, unsigned int>> samplesAnalysis;

void trialDensity(TrialPointer trial, std::string path) {
#pragma mark - Image reading

        brahand::Image image;
        brahand::Profile<>::time("Opening " + trial->folderPath + trial->imageName, [&](){ image = brahand::Image(trial->folderPath + trial->imageName); });
//        brahand::VTKExport::image(image, path, "image");

#pragma mark - Edges

        brahand::Image edgesImage;
        brahand::Profile<>::time("Edges", [&](){ edgesImage = image.edges(trial->isovalues); });
        edgesImage.addFrame(); // Add square/cube frame

        brahand::IndicesArray edgeIndicesArray = edgesImage.features().first;
        brahand::IndicesArray gammaIndicesArray = edgesImage.features().second;

        brahand::VTKExport::coordinates(edgeIndicesArray, image.size, path, "edgeCoordinates");

#pragma mark - Gradient Flux Skeleton

        brahand::Image skeletonImage; brahand::ImageWrapper<float> dt;
        brahand::Profile<>::time("Skeleton", [&](){ skeletonImage = edgesImage.jacobianSkeleton(dt, trial->blur, trial->flux); });
        brahand::IndicesArray skeletonIndicesArray = skeletonImage.features().first;

        brahand::VTKExport::coordinates(skeletonIndicesArray, image.size, path, "skeletonCoordinates");
        printf("\nDT min: %f, max: %f\n",dt.imagePointer.minMax().first, dt.imagePointer.minMax().second);

#pragma mark - Edge density

        brahand::ImageWrapper<float> density;
        brahand::Profile<>::time("Edge Density", [&](){ density = image.edgeDensity(edgeIndicesArray, skeletonIndicesArray); });

        brahand::VTKExport::density(density, image.size, path, "edgeDensity");

#pragma mark - Propagation

        brahand::Profile<>::time("Propagation", [&](){ brahand::Image::propagation(density, dt, edgeIndicesArray, trial->waveLength); });

        brahand::VTKExport::image<float>(dt, path, "dt");
        brahand::VTKExport::image(density, path, "densityMap");


        float bounds;
        std::vector<Resume> results;

        if(image.size.depth > 1){
            bounds = std::min(std::min(image.size.width, image.size.height), image.size.depth );
        } else {
            bounds = std::min(image.size.width, image.size.height);
        }
        bounds /= 4;

        path = RESULTS_PATH + trial->identifier + "/density-trials/";
        mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );

        float ratio = 1.0;
        uint trialNumber = 0;
        float initMin = 1.0;
        for( float minDensity = initMin ; minDensity < bounds ; minDensity += ratio ) {
            for( float maxDensity = minDensity ; maxDensity < bounds ; maxDensity += ratio ) {
                ++trialNumber;
                printf("\n%d %f %f \n", trialNumber, minDensity, maxDensity);

                brahand::ImageWrapper<float> normalizedDensity = density.normalize<float>(minDensity, maxDensity);

#pragma mark - Poisson sampling

                brahand::Poisson sampler( image.size, normalizedDensity );
                brahand::IndicesArray edgeSamples, samples;

                brahand::Profile<>::time("Edge Samples", [&](){ edgeSamples = sampler.sampling( edgeIndicesArray ); });
                brahand::Profile<>::time("Samples", [&](){ samples = sampler.sampling(gammaIndicesArray); });

                printf("Total samples: %d\n", samples.count);
                brahand::VTKExport::coordinates(edgeSamples, image.size, path, "edgeSamples" + std::to_string(trialNumber) );
                brahand::VTKExport::coordinates(samples, image.size, path, "samples" + std::to_string(trialNumber) );

#pragma mark - Triangulation

                Histogram histogram;
                std::shared_ptr<CGALTriangulation> triangulation;

                brahand::Profile<>::time("Triangulation", [&](){
                    if(image.size.depth == 1){ triangulation = std::make_shared<CGALTriangulation2D>( samples, image.size);
                    } else { triangulation = std::make_shared<CGALTriangulation3D>( samples, image.size); }
                });

                histogram = triangulation->generate(path, "triangulation" + std::to_string(trialNumber) , image);
                std::vector<uint> h = saveHistogram(path, "histogram" + std::to_string(trialNumber) + ".txt", histogram);

                results.push_back({ trialNumber, minDensity, maxDensity, samples.count, h, {initMin,bounds},{initMin,bounds} });
            }
        }

        path = RESULTS_PATH + trial->identifier + "/";

        std::ofstream os;
        os.open(path + trial->identifier + ".txt");
        os << results[0].minRange.first << "\t" << results[0].minRange.second << "\n";
        os << results[0].maxRange.first << "\t" << results[0].maxRange.second << "\n";
        os << initMin << "\t" << ratio << "\t" << bounds << "\n";

        for( uint i = 0 ; i < results.size() ; ++i) {
            os << results[i].id << "\t" << results[i].samples << "\t" << results[i].minDensity << "\t" << results[i].maxDensity;
            histogramAnalysis[results[i].minDensity][results[i].maxDensity] = results[i].histogram;
            histogramAnalysis[results[i].maxDensity][results[i].minDensity] = results[i].histogram;
            samplesAnalysis[results[i].minDensity][results[i].maxDensity] = results[i].samples;
            samplesAnalysis[results[i].maxDensity][results[i].minDensity] = results[i].samples;
            for( auto f : results[i].histogram) { os << "\t" << f; }
            os << "\n";
        }
        os.close();

        std::ofstream os_x; os_x.open(path + trial->identifier + "_x.txt");
        for( float minDensity = initMin ; minDensity < bounds ; minDensity += ratio ) {
            for( float maxDensity = initMin ; maxDensity < bounds ; maxDensity += ratio ) {
                os_x << maxDensity << " ";
            }
            os_x << "\n";
        }
        os_x.close();

        std::ofstream os_y; os_y.open(path + trial->identifier + "_y.txt");
        for( float minDensity = initMin ; minDensity < bounds ; minDensity += ratio ) {
            for( float maxDensity = initMin ; maxDensity < bounds ; maxDensity += ratio ) {
                os_y << minDensity << " ";
            }
            os_y << "\n";
        }
        os_y.close();

        std::ofstream os_avg; os_avg.open(path + trial->identifier + "_average.txt");
        std::ofstream os_sum; os_sum.open(path + trial->identifier + "_sumatory.txt");
        std::ofstream os_min; os_min.open(path + trial->identifier + "_minAngle.txt");
        std::ofstream os_sam; os_sam.open(path + trial->identifier + "_samples.txt");

        for( float i = initMin ; i < bounds ; i += ratio ) {
            for( float j = initMin ; j < bounds ; j += ratio ) {
                if(histogramAnalysis.find(i) != histogramAnalysis.end()){
                    if(histogramAnalysis[i].find(j) != histogramAnalysis[i].end()){
                        int minAngle = 0;
                        for(int k = 0 ; k < (int)histogramAnalysis[i][j].size() ; ++k){
                            if(histogramAnalysis[i][j][k] > 0){ minAngle = k; break; }
                        }

                        uint weight = 1;
                        uint sumatory = 0;
                        for(int k = 0 ; k < (int)histogramAnalysis[i][j].size() ; ++k, ++weight){
                            sumatory += histogramAnalysis[i][j][k] * weight;
                        }
                        float average =  sumatory / histogramAnalysis[i][j].size();

                        os_avg << average << " ";
                        os_sum << sumatory << " ";
                        os_min << minAngle << " ";
                        os_sam << samplesAnalysis[i][j] << " ";
                    }
                    else{
                        std::cout << i << " " << j << "\n";
                        os_avg << 0.0 << " "; os_sum << 0.0 << " "; os_min << 0.0 << " "; os_sam << 0.0 << " ";
                    }
                }
                else{
                    std::cout << i << " " << j << "\n";
                    os_avg << 0.0 << " "; os_sum << 0.0 << " "; os_min << 0.0 << " "; os_sam << 0.0 << " ";
                }
            }
            os_avg << "\n"; os_sum << "\n"; os_min << "\n"; os_sam << "\n";
        }
        os_avg.close(); os_sum.close(); os_min.close(); os_sam.close();
}


#endif // TRIALDENSITY_H
