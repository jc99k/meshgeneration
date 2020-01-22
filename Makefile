main:
	mkdir -p build
	g++ -c -pipe -frounding-math -O3 -fopenmp -Wno-unknown-pragmas -O2 -march=x86-64 -mtune=generic -O2 -pipe -fstack-protector-strong -fno-plt -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -IB-Mesh -I. -o build/main.o B-Mesh/main.cpp
	g++ -fopenmp -o build/BMesh build/main.o -lCGAL -lgmp -lmpfr -lpthread
	rm build/*.o

clean:
	rm -rf build/*
	rm -rf Results/*

rebuild: clean main