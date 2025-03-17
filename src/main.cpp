#include <iostream>
#include <fcntl.h> //file handling
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <semaphore.h>
#include "Aircraft.h"
#include <pthread.h>
using namespace std;


void* Aircraft_Init(void *arg){
	Aircraft air= Aircraft();

	air.StartTimer();
	return nullptr;
}


int main() {
	pthread_t threads;


	if (pthread_create(&threads, nullptr, &Aircraft_Init, nullptr) != 0) {
		cerr << "Error creating thread " << endl;
		return 1;
	}
	pthread_join(threads,nullptr);



    return 0;
}
