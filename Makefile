main:
	mkdir -p build
	g++ -fopenmp -std=c++11 -IB-Mesh -I. -lCGAL -lgmp -lmpfr -lpthread -o build/BMesh B-Mesh/main.cpp B-Mesh/Yaml.cpp

clean:
	rm -rf build/*
	rm -rf Results/*

rebuild: clean main