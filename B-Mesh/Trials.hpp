//
//  Trials.hpp
//  B-Mesh
//
//  Created by Bryan Gonzales Vega on 6/14/16.
//  Copyright © 2016 Bryan Gonzales Vega. All rights reserved.
//

#ifndef Trials_hpp
#define Trials_hpp

#include <string>
#include <memory>
#include "BMesh/Image.hpp"

#define BUILD_TRIAL(DerivedTrial) std::make_shared<DerivedTrial>()

struct Trial{
    std::string identifier;
    std::string folderPath;
    std::string imageName;
    
    brahand::ImageIsovaluesVector isovalues;
    brahand::uint waveLength;
    
    float flux;
    
    float minDensity;
    float maxDensity;
    
};

typedef std::shared_ptr<Trial> TrialPointer;

struct kMesh1 : Trial {
    kMesh1(){
        identifier  = "kmesh1";
        folderPath  = ASSETS_PATH;
        imageName   = "kmesh1copy.png";
        isovalues   = {49,50,99,100};
        waveLength  = 500;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 150;
    }
};

struct kMesh2 : Trial {
    kMesh2(){
        identifier  = "kmesh2";
        folderPath  = ASSETS_PATH;
        imageName   = "kmesh2.jpg";
        isovalues   = {50,80,100,150,200};
        waveLength  = 700;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 150;
    }
};

struct Proposal : Trial {
    Proposal(){
        identifier  = "proposal";
        folderPath  = ASSETS_PATH;
        imageName   = "proposal.png";
        isovalues   = {200};
        waveLength  = 200;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 60;
    }
};

struct Square : Trial {
    Square(){
        identifier  = "square";
        folderPath  = ASSETS_PATH;
        imageName   = "square.jpg";
        isovalues   = {200};
        waveLength  = 200;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 60;
    }
};

struct Taz : Trial {
    Taz(){
        identifier  = "taz";
        folderPath  = ASSETS_PATH;
        imageName   = "taz.jpg";
        isovalues   = {140};
        waveLength  = 100;
        flux        = -0.241867;
        minDensity  = 2;
        maxDensity  = 40.0;
    }
};

struct Titicaca : Trial {
    Titicaca(){
        identifier  = "titicaca";
        folderPath  = ASSETS_PATH;
        imageName   = "titicaca-lake.jpg";
        isovalues   = {80};
        waveLength  = 240;
        flux        = -0.3;
        minDensity  = 3.0;
        maxDensity  = 70;
    }
};

struct Aneurism : Trial {
    Aneurism(){
        identifier  = "aneurism";
        folderPath  = ASSETS_PATH;
        imageName   = "aneurism.png";
        isovalues   = {5};
        waveLength  = 162;
        flux        = -0.3;
        minDensity  = 5.0;
        maxDensity  = 50;
    }
};

struct EngineSlice : Trial {
    EngineSlice(){
        identifier  = "engineSlice";
        folderPath  = ASSETS_PATH;
        imageName   = "engineSlice.pgm";
        isovalues   = {100};
        waveLength  = 53;
        flux        = -0.3;
        minDensity  = 2;
        maxDensity  = 17;
    }
};

struct Fire : Trial {
    Fire(){
        identifier  = "fire";
        folderPath  = ASSETS_PATH;
        imageName   = "fire.png";
        isovalues   = {100};
        waveLength  = 110;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 40;
    }
};


struct Circles : Trial {
    Circles(){
        identifier  = "circles";
        folderPath  = ASSETS_PATH;
        imageName   = "circles.png";
        isovalues   = {200};
        waveLength  = 200;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 60;
    }
};

struct MachuPicchu : Trial {
    MachuPicchu(){
        identifier  = "machuPicchu";
        folderPath  = ASSETS_PATH;
        imageName   = "machupicchu.jpg";
        isovalues   = {80};
        waveLength  = 200;
        flux        = -0.3;
        minDensity  = 5;
        maxDensity  = 100;
    }
};

struct Lion : Trial {
    Lion(){
        identifier  = "lion";
        folderPath  = ASSETS_PATH;
        imageName   = "lion.jpg";
        isovalues   = {100};
        waveLength  = 400;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 70;
    }
};





struct Buddha : Trial {
    Buddha(){
        identifier  = "buddha";
        folderPath  = ASSETS_PATH;
        imageName   = "buddha";
        isovalues   = {21};
        waveLength  = 83;
        flux        = -0.299;
        minDensity  = 1.2;
        maxDensity  = 26.0;
    }
};

struct Carp : Trial {
    //    Flux: min: -0.657721	 max:0.330319
    Carp(){
        identifier  = "carp";
        folderPath  = ASSETS_PATH;
        imageName   = "carp";
        isovalues   = {200,100};
        waveLength  = 101;
        flux        = -0.3;
        minDensity  = 2;
        maxDensity  = 26.0;
    }
};


struct Aneurism3d : Trial {
    //    Flux: min: -0.657721	 max:0.330319
    Aneurism3d(){
        identifier  = "aneurism";
        folderPath  = ASSETS_PATH;
        imageName   = "aneurism";
        isovalues   = {254};
        waveLength  = 68;
        flux        = -0.154;
        minDensity  = 2;
        maxDensity  = 20.0;
    }
};

struct MouseLvb : Trial {
    //    Flux: min: -0.657721	 max:0.330319
    MouseLvb(){
        identifier  = "mouseLvb";
        folderPath  = ASSETS_PATH;
        imageName   = "mouse_lvb";
        isovalues   = {150};
        waveLength  = 86;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 30;
    }
};


struct Hyena : Trial {
    Hyena(){
        identifier  = "hyena";
        folderPath  = ASSETS_PATH;
        imageName   = "hyena";
        isovalues   = {60,10};
        waveLength  = 113;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 40.0;
    }
};

struct Chest : Trial {
    Chest(){
        identifier  = "chest";
        folderPath  = ASSETS_PATH;
        imageName   = "chest";
        isovalues   = {120,50};
        waveLength  = 73;
        flux        = -0.3;
        minDensity  = 2;
        maxDensity  = 25.0;
    }
};

struct Knee : Trial {
    Knee(){
        identifier  = "knee";
        folderPath  = ASSETS_PATH;
        imageName   = "knee";
        isovalues   = {60,10};
        waveLength  = 76; // 280
        flux        = -0.3;
        minDensity  = 2;
        maxDensity  = 25.0;
    }
};

struct Pig : Trial {
    Pig(){
        identifier  = "pig";
        folderPath  = ASSETS_PATH;
        imageName   = "pig";
        isovalues   = {130};
        waveLength  = 68;
        flux        = -0.3;
        minDensity  = 2.0;
        maxDensity  = 20.0;
    }
};


struct Foot : Trial {
    Foot(){
        identifier  = "foot";
        folderPath  = ASSETS_PATH;
        imageName   = "foot";
        isovalues   = {8};
        waveLength  = 67;
        flux        = -0.3;
        minDensity  = 2.0;
        maxDensity  = 20.0;
    }
};

struct Bonsai : Trial {
    Bonsai(){
        identifier  = "bonsai";
        folderPath  = ASSETS_PATH;
        imageName   = "bonsai";
        isovalues   = {9};
        waveLength  = 78;
        flux        = -0.2389;
        minDensity  = 3;
        maxDensity  = 30;
    }
};

struct Engine : Trial {
    //    -0.732903	 max:0.330319
    Engine(){
        identifier  = "engine";
        folderPath  = ASSETS_PATH;
        imageName   = "engine";
        isovalues   = {101};
        waveLength  = 55;
        flux        = -0.2813;
        minDensity  = 2;
        maxDensity  = 20.0;
    }
};
struct Hydrogen : Trial {
    // Flux: min: -0.660510	 max:0.243475
    Hydrogen(){
        identifier  = "hydrogen";
        folderPath  = ASSETS_PATH;
        imageName   = "hydrogen";
        isovalues   = {26};
        waveLength  = 39;
        flux        = -0.253;
        minDensity  = 0.8;
        maxDensity  = 6;
    }
};

struct Dragon : Trial {
    Dragon(){
        identifier  = "dragon";
        folderPath  = ASSETS_PATH;
        imageName   = "dragon";
        isovalues   = {16};
        waveLength  = 91;
        flux        = -0.2454;
        minDensity  = 2;
        maxDensity  = 25.0;
    }
};














































/////////////////////////////////// PARAMETERS ///////////////////////////////////

struct Leaf1P : Trial {
    Leaf1P(){
        identifier  = "leaf1";
        folderPath  = ASSETS_PATH;
        imageName   = "leaf.jpg";
        isovalues   = {200};
        waveLength  = 445;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 40;
    }
};
struct Leaf2P : Trial {
    Leaf2P(){
        identifier  = "leaf2";
        folderPath  = ASSETS_PATH;
        imageName   = "leaf.jpg";
        isovalues   = {123};
        waveLength  = 445;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 40;
    }
};
struct LeafP : Trial {
    LeafP(){
        identifier  = "leaf";
        folderPath  = ASSETS_PATH;
        imageName   = "leaf.jpg";
        isovalues   = {200,123};
        waveLength  = 445;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 40;
    }
};



struct EngineSlice1P : Trial {
    EngineSlice1P(){
        identifier  = "engineSlice1";
        folderPath  = ASSETS_PATH;
        imageName   = "engineSlice.pgm";
        isovalues   = {100};
        waveLength  = 200;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 20;
    }
};
struct EngineSlice2P : Trial {
    EngineSlice2P(){
        identifier  = "engineSlice2";
        folderPath  = ASSETS_PATH;
        imageName   = "engineSlice.pgm";
        isovalues   = {200};
        waveLength  = 200;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 20;
    }
};
struct EngineSliceP : Trial {
    EngineSliceP(){
        identifier  = "engineSlice";
        folderPath  = ASSETS_PATH;
        imageName   = "engineSlice.pgm";
        isovalues   = {100,200};
        waveLength  = 200;
        flux        = -0.3;
        minDensity  = 3;
        maxDensity  = 20;
    }
};




struct Chest1 : Trial {
    Chest1(){
        identifier  = "chest1";
        folderPath  = ASSETS_PATH;
        imageName   = "chest";
        isovalues   = {120};
        waveLength  = 380;
        flux        = -0.5;
        minDensity  = 2;
        maxDensity  = 60;
    }
};
struct Chest2 : Trial {
    Chest2(){
        identifier  = "chest2";
        folderPath  = ASSETS_PATH;
        imageName   = "chest";
        isovalues   = {50};
        waveLength  = 380;
        flux        = -0.5;
        minDensity  = 2;
        maxDensity  = 60;
    }
};
struct ChestP : Trial {
    ChestP(){
        identifier  = "chestP";
        folderPath  = ASSETS_PATH;
        imageName   = "chest";
        isovalues   = {120,50};
        waveLength  = 380;
        flux        = -0.5;
        minDensity  = 2;
        maxDensity  = 60;
    }
};



struct Carp1 : Trial {
    //    Flux: min: -0.657721	 max:0.330319
    Carp1(){
        identifier  = "carp1";
        folderPath  = ASSETS_PATH;
        imageName   = "carp";
        isovalues   = {200};
        waveLength  = 380;
        flux        = -0.5;
        minDensity  = 2;
        maxDensity  = 60;
    }
};
struct Carp2 : Trial {
    //    Flux: min: -0.657721	 max:0.330319
    Carp2(){
        identifier  = "carp2";
        folderPath  = ASSETS_PATH;
        imageName   = "carp";
        isovalues   = {100};
        waveLength  = 380;
        flux        = -0.5;
        minDensity  = 2;
        maxDensity  = 60;
    }
};
struct CarpP : Trial {
    //    Flux: min: -0.657721	 max:0.330319
    CarpP(){
        identifier  = "carpP";
        folderPath  = ASSETS_PATH;
        imageName   = "carp";
        isovalues   = {200,100};
        waveLength  = 380;
        flux        = -0.5;
        minDensity  = 2;
        maxDensity  = 60;
    }
};























//struct Leaf : Trial {
//    Leaf(){
//        identifier  = "leaf";
//        folderPath  = ASSETS_PATH;
//        imageName   = "leaf.jpg";
//        isovalues   = {200,125};
//        waveLength  = 445;
//        blur        = 0.1;
//        flux        = -0.3;
//        minDensity  = 3;
//        maxDensity  = 40;
//    }
//};

//struct Leaf2 : Trial {
//    Leaf2(){
//        identifier  = "leaf2";
//        folderPath  = ASSETS_PATH;
//        imageName   = "leaf2.png";
//        isovalues   = {200};
//        waveLength  = 300;
//        blur        = 0.1;
//        flux        = -0.3;
//        minDensity  = 3;
//        maxDensity  = 40;
//    }
//};

struct Neghip : Trial {
    Neghip(){
        identifier  = "neghip";
        folderPath  = ASSETS_PATH;
        imageName   = "neghip";
        isovalues   = {20};
        waveLength  = 45;
        minDensity  = 2.0;
        maxDensity  = 20.0;
    }
};

struct Monkey : Trial {
    Monkey(){
        identifier  = "monkey";
        folderPath  = ASSETS_PATH;
        imageName   = "monkey";
        isovalues   = {20};
        waveLength  = 45;
        minDensity  = 2.0;
        maxDensity  = 20.0;
    }
};


struct Mata : Trial {
    Mata(){
        identifier  = "mata";
        folderPath  = ASSETS_PATH;
        imageName   = "mata";
        isovalues   = {2, 150};
        waveLength  = 280;
        flux        = -0.3;
        minDensity  = 5;
        maxDensity  = 50.0;
    }
};






#endif /* Trials_h */
