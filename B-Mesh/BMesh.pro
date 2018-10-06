#-lm -lpthread -lX11

CONFIG  += c++11

INCLUDEPATH += "/usr/local/cuda-8.0/include"

LIBS    += -lCGAL #cgal
LIBS    += -lgmp #cgal
LIBS    += -lmpfr #cgal
LIBS    += -L/usr/lib64 -lOpenCL #opencl

QMAKE_CXXFLAGS  += -frounding-math -O3 #cgal
QMAKE_CXXFLAGS  += -fopenmp #openmp
QMAKE_CXXFLAGS  += -Wno-unknown-pragmas # disable warnings for #pragma mark
QMAKE_LFLAGS    += -fopenmp #openmp

SOURCES += \
    main.cpp \

HEADERS += \
    Trials.hpp \
    MeshGeneration.h \
    TrialDensity.h
