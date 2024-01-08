all:
	g++ -O2 -march=native -lpthread main.cc

debug:
	g++ -g -lpthread main.cc