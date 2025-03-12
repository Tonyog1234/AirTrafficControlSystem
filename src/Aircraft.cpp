#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <fstream>
#include <csignal>
#include <ctime>
#include <vector>
#include <fcntl.h>      // For shm_open
#include <sys/mman.h>   // For mmap
#include <cstring>      // For memcpy
#include "Aircraft.h"
using namespace std;
static streampos lastPosition = 0;

Aircraft::Aircraft(){
	// Initialize shared memory
	    const char* SHM_NAME = "/aircraft_shm";
	    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
	    if (shm_fd == -1) {
	        perror("shm_open failed");
	        exit(1);
	    }

	    // Set the size of the shared memory
	    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
	        perror("ftruncate failed");
	        exit(1);
	    }

	    // Map the shared memory
	    shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	    if (shm_ptr == MAP_FAILED) {
	        perror("mmap failed");
	        exit(1);
	    }
	    // Initialize shared memory with count = 0
	        *(static_cast<int*>(shm_ptr)) = 0;

	ifstream input("input.txt");
	//exception handling
	try{
		if(!input){
			perror("Error opening input.txt");
			exit(1);
		}
		else{

			while (input >> id >> x >> y >> z >> speedX >> speedY >> speedZ) {
			            airplane.emplace_back(id, x, y, z, speedX, speedY, speedZ);
			            data.push_back({id, x, y, z, speedX, speedY, speedZ});
			        }
		}
	}
	catch(...){
		cout<<"Exception! Check Input file"<<endl;
	}
	input.close();

	CheckAirSpace();

	*(static_cast<int*>(shm_ptr)) = data.size();//return number of objects

	//void* memcpy(void* dest, const void* src, size_t n)
	memcpy(static_cast<char*>(shm_ptr) + sizeof(int), data.data(), data.size() * sizeof(AircraftData));
}
    //Constructor
Aircraft::Aircraft(int id, double x, double y, double z, double speedX, double speedY, double speedZ){
	this->id=id;
	this->x=x;
	this->y=y;
	this->z=z;
	this->speedX=speedX;
	this->speedY=speedY;
	this->speedZ=speedZ;
	status=true;

 }
//Destructor
Aircraft::~Aircraft(){
	/*if (shm_ptr != nullptr && shm_ptr != MAP_FAILED) {
	        munmap(shm_ptr, SHM_SIZE);
	    }
	    if (shm_fd != -1) {
	        close(shm_fd);
	        // Uncomment to clean up shared memory (only one process should do this)
	        // shm_unlink("/aircraft_shm");
	    }*/

}
    //Getter
int Aircraft::getID() const{
	return id;

}

double Aircraft::getX() const{
	return x;
}
double Aircraft::getY() const{
	return y;
}
double Aircraft::getZ() const{
	return z;
}

double Aircraft::getSpeedX() const{
	return speedX;
}
double Aircraft::getSpeedY() const{
	return speedY;
}
double Aircraft::getSpeedZ() const{
	return speedZ;
}
bool Aircraft::getStatus() const{
	return status;
}

    //Setter
void Aircraft::setID(int id) {
	this->id=id;
}

void Aircraft::setX(double x) {
	this->x=x;
}
void Aircraft::setY(double y) {
	this->y=y;
}
void Aircraft::setZ(double z) {
	this->z=z;
}

void Aircraft::setSpeedX(double speedX) {
	this->speedX=speedX;
}
void Aircraft::setSpeedY(double speedY) {
	this->speedY=speedY;
}
void Aircraft::setSpeedZ(double speedZ) {
	this->speedZ=speedZ;
}
void Aircraft::setStatus(bool status){
	this->status=status;
}

    //Update
void Aircraft::UpdatePosition(){
	this->x+= speedX;
	this->y+= speedY;
	this->z+= speedZ;
	CheckAirSpace();
	print();
	//update to share memory


	   // *(static_cast<int*>(shm_ptr)) = data.size();
	   // memcpy(static_cast<char*>(shm_ptr) + sizeof(int), data.data(), data.size() * sizeof(AircraftData));

}
void Aircraft::CheckAirSpace(){
	if (x < 0 || x > 100000 || y < 0 || y > 100000 || z < 15000 || z > 40000) {
		            status = false; // Exit airspace
		        }
}
    //Print
void Aircraft::print(){
	cout<<"Flight ID: "<< id <<endl;
	cout<<"Flight Position: ("<<x<<", "<<y<<", "<<z<<")"<<endl;
	cout<<"Flight Speed: ("<<speedX<<", "<<speedY<<", "<<speedZ<<")"<<endl;
	cout<<"****************************"<<endl;

}

//Timer
void Aircraft::TimerHandler(union sigval sv) {
    Aircraft* aircraft = static_cast<Aircraft*>(sv.sival_ptr);// cast sival_ptr to Aircraft*
    /*
     * union sigval {
    int sival_int;    // Integer value
    void *sival_ptr;  // Pointer value
};
     * */
    for(size_t i=0; i<aircraft->airplane.size(); i++){
    	aircraft->airplane[i].UpdatePosition();
    }

}

void Aircraft::StartTimer() {
    timer_t timer_id;
    struct sigevent sev;
    struct itimerspec its;

    // Set up the sigevent for the timer
    sev.sigev_notify = SIGEV_THREAD;           // Notify via thread
    sev.sigev_notify_function = TimerHandler;  // Now works because TimerHandler is static
    sev.sigev_value.sival_ptr = this;          // Pass the instance pointer
    sev.sigev_notify_attributes = nullptr;

    // Create the timer
    if (timer_create(CLOCK_REALTIME, &sev, &timer_id) == -1) {
        std::cerr << "Error creating timer: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set the timer to expire every 2 seconds
    its.it_value.tv_sec = 1;    // First expiration after 1 seconds
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 3; // Repeat every 3 seconds
    its.it_interval.tv_nsec = 0;

    // Start the timer
    if (timer_settime(timer_id, 0, &its, nullptr) == -1) {
        std::cerr << "Error setting timer: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    while(1) {
        sleep(1);
    }
}
