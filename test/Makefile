all: bench_client.cc bench_common.h bench_config.h 
	g++ -std=c++11 bench_client.cc -I../ ../MurmurHash3.cpp -o ycsb_test

clean:
	rm ycsb_test

