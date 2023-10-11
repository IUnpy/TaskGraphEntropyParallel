final: permutation_entropy.o pec_starpu_main.o 
	g++ -o final  permutation_entropy.o  pec_starpu_main.o  `pkg-config starpu-1.3 --libs --cflags` -mcmodel=medium  -pthread 
	
permutation_entropy.o: permutation_entropy.cpp
	g++ -c permutation_entropy.cpp  `pkg-config starpu-1.3  --libs --cflags` -mcmodel=medium  -pthread  

pec_starpu_main.o: pec_starpu_main.cpp
	g++ -c pec_starpu_main.cpp  `pkg-config starpu-1.3  --libs --cflags` -mcmodel=medium  -pthread 

clean:
