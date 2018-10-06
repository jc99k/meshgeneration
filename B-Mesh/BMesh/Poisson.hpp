#include "Modules/Essentials/Essentials.hpp"

#include <assert.h>
#include <math.h>
#include <random>

namespace brahand{

    const std::vector<std::vector<int>> neighbors2DMap {
    	{-2,-2,0},{-2,-1,0},{-2,0,0},{-2,1,0},{-1,2,0},
    	{-1,-2,0},{-1,-1,0},{-1,0,0},{-1,1,0},{-1,2,0},
    	{0,-2,0},{0,-1,0},{0,0,0},{0,1,0},{0,2,0},
    	{1,-2,0},{1,-1,0},{1,0,0},{1,1,0},{1,2,0},
    	{2,-2,0},{2,-1,0},{2,0,0},{2,1,0},{1,2,0}
    };

    const std::vector<std::vector<int>> neighbors3DMap {
    	{-2,-2,-1},{-2,-1,-1},{-2,0,-1},{-2,1,-1},{-1,2,-1},
    	{-1,-2,-1},{-1,-1,-1},{-1,0,-1},{-1,1,-1},{-1,2,-1},
    	{0,-2,-1},{0,-1,-1},{0,0,-1},{0,1,-1},{0,2,-1},
    	{1,-2,-1},{1,-1,-1},{1,0,-1},{1,1,-1},{1,2,-1},
    	{2,-2,-1},{2,-1,-1},{2,0,-1},{2,1,-1},{1,2,-1},

    	{-2,-2,0},{-2,-1,0},{-2,0,0},{-2,1,0},{-1,2,0},
    	{-1,-2,0},{-1,-1,0},{-1,0,0},{-1,1,0},{-1,2,0},
    	{0,-2,0},{0,-1,0},{0,0,0},{0,1,0},{0,2,0},
    	{1,-2,0},{1,-1,0},{1,0,0},{1,1,0},{1,2,0},
    	{2,-2,0},{2,-1,0},{2,0,0},{2,1,0},{1,2,0},

    	{-2,-2,1},{-2,-1,1},{-2,0,1},{-2,1,1},{-1,2,1},
    	{-1,-2,1},{-1,-1,1},{-1,0,1},{-1,1,1},{-1,2,1},
    	{0,-2,1},{0,-1,1},{0,0,1},{0,1,1},{0,2,1},
    	{1,-2,1},{1,-1,1},{1,0,1},{1,1,1},{1,2,1},
    	{2,-2,1},{2,-1,1},{2,0,1},{2,1,1},{1,2,1}
    };

    template<class T>
    inline float euclideanDistance ( T ax, T ay, T az, T bx, T by, T bz) {
        float xx = (ax - bx) * (ax - bx);
        float yy = (ay - by) * (ay - by);
        float zz = (az - bz) * (az - bz);
        return std::sqrt( xx + yy + zz );
    }


    struct Grid {
    	short rows, cols, layers;
    	std::vector< std::vector< brahand::uint> > cells;

    	Grid() = default;

    	std::vector<brahand::uint>& operator()(short i, short j, short k){
            assert(i >= 0 && i < rows && j >= 0 && j < cols && k >= 0 && k < layers);
    		brahand::uint index = (k * cols  * rows) + (i * cols) + j;
    		return cells[index];
    	}

    	void reserve(short rows, short cols, short layers){
    		this->rows	= rows;
    		this->cols	= cols;
    		this->layers = layers;
    		cells		= std::vector< std::vector<uint> >( this->rows * this->cols * this->layers );
    	}

        void print(){
            printf("Grid (%d,%d,%d)\n", rows, cols, layers);
            for(int i = 0 ; i < cells.size() ; ++i){
                printf("[%d: %lu]: ", i, cells[i].size() );
                for( int j = 0 ; j < cells[i].size() ; ++j){
                    std::cout << cells[i][j] << " ";
                }
                printf("\n");
            }

        }
    };

    struct EuclideanCoordinate{
        float x,y,z;
    };

    template <class T>
    T chooseRandom(T lower, T upper, std::mt19937 randomEngine ){
    	std::uniform_int_distribution<T> processingListDistribution(lower, upper);
    	return processingListDistribution(randomEngine);
    }



    class PoissonSampler {
    private:
        brahand::ImageSize size;
        brahand::IndicesArray coordinates;
        float cellSize;
        uint k;
        ImageWrapper<float> density;

        Grid poissonGrid, imageGrid;
        std::vector<uint> validCellsIds;
        std::vector<uint> processingList, outputList;

        std::random_device randomDevice;
        std::mt19937 randomEngine{randomDevice()};

        EuclideanCoordinate imageCoordinateToCellCoordinate(uint coordinate) {
            uint idx, row, col, layer;
            idx = coordinate;
            layer = idx / (size.width * size.height); // z
            idx -= (layer * size.width * size.height);
            row = idx / size.width; // y
            col = idx % size.width; // x

            assert(col >= 0 && col < this->size.width && row >= 0 && row < this->size.height && layer >= 0 && layer < this->size.depth);

            float i = (float) (floorf((float)row / this->cellSize));
            float j = (float) (floorf((float)col / this->cellSize));
            float k = (float) (floorf((float)layer / this->cellSize));

            return EuclideanCoordinate{i,j,k};
        }

    public:
        PoissonSampler() = delete;
        PoissonSampler(brahand::ImageSize size, float cellSize, uint k, brahand::IndicesArray coordinates, ImageWrapper<float> density, brahand::IndicesArray existingSamples){

            this->k = k;
            this->density = density;
            this->cellSize = cellSize;
            this->size = size;
            this->coordinates = coordinates;

            short gridCols = ceilf((float)this->size.width / cellSize);
            short gridRows = ceilf((float)this->size.height / cellSize);
            short gridLayers = ceilf((float)this->size.depth / cellSize);

            this->poissonGrid.reserve(gridRows, gridCols, gridLayers);
    		this->imageGrid.reserve(gridRows, gridCols, gridLayers);

            /// Read EuclideanCoordinates and assign it to imageGrid;
    		for(uint i = 0 ; i < coordinates.count ; ++i){
                auto coordinateIndex = this->coordinates[i];
    			auto gridCoordinate = imageCoordinateToCellCoordinate(coordinateIndex);
    			this->imageGrid(gridCoordinate.x, gridCoordinate.y, gridCoordinate.z).push_back(coordinateIndex);
    		}
            // imageGrid.print();

            /// Get index of valid cells (non-empty) from imageGrid
    		for (uint imgIndex = 0 ; imgIndex < this->imageGrid.cells.size() ; ++imgIndex) {
    			if( this->imageGrid.cells[imgIndex].size() > 0 ){ this->validCellsIds.push_back(imgIndex); }
    		}

            for(uint i = 0 ; i < existingSamples.count ; ++i ){
                this->outputList.push_back(existingSamples[i]);
                EuclideanCoordinate gridCoordinate = imageCoordinateToCellCoordinate(existingSamples[i]);
                this->poissonGrid(gridCoordinate.x, gridCoordinate.y, gridCoordinate.z).push_back(existingSamples[i]);
            }

            uint initialPoint;

            /// Choose a random point from image's valid cells that is not in conflict with already inserted points from edge density
//            do{
//                uint randomIndex = chooseRandom<uint>(0, (uint)this->validCellsIds.size()-1, randomEngine);
//                initialPoint = imageGrid.cells[this->validCellsIds[randomIndex]][0]; // first one
//            } while( coordinateIsInConflictWithNeighbors(initialPoint, iterateNeighbors(imageCoordinateToCellCoordinate(initialPoint), this->poissonGrid) ) );

            initialPoint = imageGrid.cells[this->validCellsIds[0]][0];

    		/// Locate the choosen point into the grid coordinates.
    		EuclideanCoordinate gridCoodinate = imageCoordinateToCellCoordinate(initialPoint);

            /// Add it to grid cells, processing list and output list

    		this->processingList.push_back(initialPoint);
    		this->poissonGrid(gridCoodinate.x, gridCoodinate.y, gridCoodinate.z).push_back(initialPoint);
            this->outputList.push_back(initialPoint);

            // poissonGrid.print();
        }

        PoissonSampler(brahand::ImageSize size, float cellSize, uint k, brahand::IndicesArray coordinates, ImageWrapper<float> density){
            this->k = k;
            this->density = density;
            this->cellSize = cellSize;
            this->size = size;
            this->coordinates = coordinates;

            short gridCols = ceilf((float)this->size.width / cellSize);
            short gridRows = ceilf((float)this->size.height / cellSize);
            short gridLayers = ceilf((float)this->size.depth / cellSize);

            this->poissonGrid.reserve(gridRows, gridCols, gridLayers);
    		this->imageGrid.reserve(gridRows, gridCols, gridLayers);

            /// Read EuclideanCoordinates and assign it to imageGrid;
    		for(uint i = 0 ; i < coordinates.count ; ++i){
                auto coordinateIndex = this->coordinates[i];
    			auto gridCoordinate = imageCoordinateToCellCoordinate(coordinateIndex);

    			this->imageGrid(gridCoordinate.x, gridCoordinate.y, gridCoordinate.z).push_back(coordinateIndex);
    		}
            // imageGrid.print();

            /// Get index of valid cells (non-empty) from imageGrid
    		for (uint imgIndex = 0 ; imgIndex < this->imageGrid.cells.size() ; ++imgIndex) {
    			if( this->imageGrid.cells[imgIndex].size() > 0 ){ this->validCellsIds.push_back(imgIndex); }
    		}

            // printf("Valid cells ids (%lu): ", validCellsIds.size());
            // for(int i = 0 ;i  < validCellsIds.size() ; ++i){
            //     std::cout  << validCellsIds[i] << " ";
            // }
            // printf("\n");

            /// Choose a random point from image's valid cells
    		uint randomIndex = chooseRandom<uint>(0, (uint)this->validCellsIds.size()-1, randomEngine);
    		uint initialPoint = imageGrid.cells[this->validCellsIds[randomIndex]][0]; // first one

            // printf("Initial point: %d\n", initialPoint);

    		/// Locate the choosen point into the grid coordinates.
    		EuclideanCoordinate gridCoodinate = imageCoordinateToCellCoordinate(initialPoint);

            /// Add it to grid cells, processing list and output list
    		this->outputList.push_back(initialPoint);
    		this->processingList.push_back(initialPoint);
    		this->poissonGrid(gridCoodinate.x, gridCoodinate.y, gridCoodinate.z).push_back(initialPoint);

            // poissonGrid.print();
        }

        std::vector<uint> iterateNeighbors(EuclideanCoordinate center, Grid grid){
    		std::vector<uint> neighborsArray;

            std::vector<std::vector<int>> map = (size.depth == 1) ? neighbors2DMap : neighbors3DMap;

    		for( auto navigation :  map ){
    			int x = (int)(center.x + (int)(navigation[0]));
    			int y = (int)(center.y + (int)(navigation[1]));
    			int z = (int)(center.z + (int)(navigation[2]));

    			if( x  >=  0 && x < grid.rows && y >= 0 && y < grid.cols && z >= 0 && z < grid.layers){
    				std::vector<uint> coordinates = grid(x, y, z);
    				if( coordinates.size() > 0 ){
    					neighborsArray.insert (neighborsArray.end(), coordinates.begin(), coordinates.end());
    				}
    			}
    		}
    		return neighborsArray;
    	}

        bool coordinateIsInConflictWithNeighbors(uint coordinate, std::vector<uint> neighborsArray ){

            uint idx, row, col, layer;
            idx = coordinate;
            layer = idx / (size.width * size.height); // z
            idx -= (layer * size.width * size.height);
            row = idx / size.width; // y
            col = idx % size.width; // x

    		if(std::any_of(neighborsArray.begin() , neighborsArray.end(), [&](uint n){

                uint nidx, nrow, ncol, nlayer;
                nidx = n;
                nlayer = nidx / (size.width * size.height); // z
                nidx -= (nlayer * size.width * size.height);
                nrow = nidx / size.width; // y
                ncol = nidx % size.width; // x

                float d = brahand::euclideanDistance<float>((float)col, (float)row, (float)layer,(float)ncol, (float)nrow, (float)nlayer);

                // printf("%d [%d] - %d [%d]\n", density.propagation[coordinate].density,coordinate,  density.propagation[n].density, n);
    			return d < std::min(density.imagePointer[coordinate], density.imagePointer[n] ) ;
    		}) ){ return true; }

    		return false;
    	}


        brahand::IndicesArray sampling(){
            std::cout << "Sampling\n";
            while( this->processingList.size() > 0 ){
                printf("%d ", processingList.size());
                /// Choose a random point from the processing list.
    			uint randomIndex = chooseRandom<uint>(0, (uint)processingList.size()-1, randomEngine);

    			uint randomPoint = processingList[randomIndex];
    			EuclideanCoordinate randomPointGridCoordinate = imageCoordinateToCellCoordinate(randomPoint);

                // printf("Random point: %d: %f,%f,%f\n", randomPoint, randomPointGridCoordinate.x, randomPointGridCoordinate.y, randomPointGridCoordinate.z);

                /// For this point, generate up to k points, randomly selected from the annulus surrounding the point.
    			std::vector<uint> neighbors = iterateNeighbors(randomPointGridCoordinate, this->imageGrid);

                // printf("\t Neighbors (%lu): ", neighbors.size());
                // for(int i = 0 ; i < neighbors.size() ; ++i){
                //     std::cout << neighbors[i] << " ";
                // }
                // printf("\n");

                /// If k is not enough, change it.
                if(neighbors.size() <= this->k){ this->k = (uint)neighbors.size(); }

                /// Choose k samples from mergedArray. Unsort get k first
                std::shuffle(neighbors.begin(), neighbors.end(), randomEngine);
                std::vector<uint> kSamples;
                kSamples.insert(kSamples.end(), neighbors.begin(), neighbors.begin() + this->k);

                // printf("\t Neighbors shuffle (%lu): ", kSamples.size());
                // for(int i = 0 ; i < kSamples.size() ; ++i){
                //     std::cout << kSamples[i] << " ";
                // }
                // printf("\n");

                for(uint sample : kSamples){

    				EuclideanCoordinate sampleGridCoordinate = imageCoordinateToCellCoordinate(sample);
    				std::vector<uint> sampleNeighborsArray = iterateNeighbors(sampleGridCoordinate, this->poissonGrid);

    				if( !coordinateIsInConflictWithNeighbors(sample, sampleNeighborsArray) ){
                        // printf(">> %d\n", sample);
    					this->outputList.push_back(sample);
    					this->processingList.push_back(sample);
    					this->poissonGrid(sampleGridCoordinate.x, sampleGridCoordinate.y, sampleGridCoordinate.z ).push_back(sample);
    				}
    			}
    			processingList.erase(processingList.begin() + randomIndex);
            }

            brahand::IndicesArray samples =  brahand::IndicesArray((uint)outputList.size());
    		for(uint i = 0 ; i < outputList.size() ; ++i){ samples[i] = outputList[i]; }
    		return samples;
        }
    };
}
