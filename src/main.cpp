#include <iostream>
#include <fcntl.h> //file handling
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <semaphore.h>
#include "ShareMemory.h"
#include "Aircraft.h"
#include <pthread.h>
using namespace std;


void* Aircraft_Init(void *arg){
	Aircraft aircraft= Aircraft();
	Aircraft** ptr= aircraft.getPtr();


	while(1){
		for (int i = 0; i < MAX_AIRCRAFT; i++) {
			ptr[i]->print();
			sleep(2);
			ptr[i]->UpdatePosition();
			sleep(2);
		}
	}
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
