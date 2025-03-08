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
	Aircraft* aircraft= new Aircraft();


	//Store data in share memory
	//ShareAircraft->Store(aircraft->getID(),aircraft->getX(),aircraft->getY(),aircraft->getZ(),
							//aircraft->getSpeedX(),aircraft->getSpeedY(),aircraft->getSpeedZ());
	while(1){
		aircraft->print();
		aircraft->UpdatePosition();
		sleep(2);
	}
	//delete aircraft;
	//delete ShareAircraft;
	return nullptr;
}


int main() {
	pthread_t threads[MAX_AIRCRAFT];

	for (int i=0; i<MAX_AIRCRAFT; i++){
		pthread_create(&threads[i],nullptr,&Aircraft_Init, nullptr);
		pthread_join(threads[i], nullptr);
	}


    return 0;
}
