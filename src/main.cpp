#include <iostream>
#include <fcntl.h> //file handling
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <semaphore.h>
#include "ShareMemory.h"
#include "Aircraft.h"
#include <pthread.h>
using namespace std;

int Count=0;

void* Aircraft_Init(void *arg){
	Aircraft* aircraft= new Aircraft();




		//if (aircraft[i].getX() < 0 || aircraft[i].getX() > 100000 || aircraft[i].getY() < 0 || aircraft[i].getY() > 100000
				//|| aircraft[i].getZ() < 15000 || aircraft[i].getZ() > 40000) {
			        //    aircraft[i].setStatus(false); // Exit airspace
			       // }
		//while(1){
		//aircraft[i].UpdatePosition(aircraft[i].getX(), aircraft[i].getY(), aircraft[i].getZ(),1,2,3);


		Count++;
		aircraft->setID(Count);
		aircraft->print();
		cout<<"Count"<<Count<<endl;
		//sleep(2);
		//

	return nullptr;
}


int main() {
	pthread_t threads[MAX_AIRCRAFT];

	for (int i=0; i<MAX_AIRCRAFT; i++){
		pthread_create(&threads[i],nullptr,&Aircraft_Init, nullptr);
		pthread_join(threads[i], nullptr);
	}

	//Aircraft plane= Aircraft(1,1000,2000,2000,10,20,30);
	//plane.print();
    return 0;
}
