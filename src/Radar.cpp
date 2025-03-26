#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <pthread.h>
#include <cmath>
#include <iomanip>
#include "Radar.h"
using namespace std;

pthread_mutex_t Radar::print_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t* Radar::shm_sem = nullptr;

Radar::Radar() {
    // Open and map shared memory
    const char* SHM_NAME = "/aircraft_shm";
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);  // Read-only access
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(1);
    }

    shm_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }

    // Initial read to populate RadarData
    ReadData();
    print();
}

Radar::~Radar() {
    if (shm_ptr != nullptr && shm_ptr != MAP_FAILED) {
        munmap(shm_ptr, SHM_SIZE);
    }
    if (shm_fd != -1) {
        close(shm_fd);
    }
    timer_delete(timer_id);  // Clean up timer
}

void Radar::ReadData() {
    // Clear existing data
    RadarData.clear();
    sem_wait(shm_sem);
    // Read the number of aircraft from shared memory
    int count = *(static_cast<int*>(shm_ptr));
    if (count < 0 || count > MAX_AIRCRAFT) {
        cerr << "Invalid aircraft count: " << count << endl;
        sem_post(shm_sem);
        return;
    }

    // Read AircraftData array from shared memory
    AircraftData* data = reinterpret_cast<AircraftData*>(static_cast<char*>(shm_ptr) + sizeof(int));
    //treat Aircraft struct as array
    for (int i = 0; i < count; i++) {
    	if(data[i].status && CalculateDistance(data[i])<= range)
        RadarData.push_back(data[i]);
    }
    sem_post(shm_sem);
}

void Radar::print() {
	pthread_mutex_lock(&print_mutex);
	//system("clear");
	//X-Y plane
	cout<<"X-Y plan view"<<endl;
	char xygrid[GridSize][GridSize];
	for(int i =0; i<GridSize; i++){
		for(int j =0; j<GridSize; j++){
			xygrid[i][j]='-';
		}
	}
	for(const auto& ad : RadarData){
		int xPos = static_cast<int>(ad.x/100000 *(GridSize-1));
		int yPos = static_cast<int>(ad.y/100000 *(GridSize-1));
		if(xPos >=0 && xPos < GridSize && yPos >=0 && yPos < GridSize){
			xygrid[GridSize-1-yPos][xPos]='0' + ad.id;
		}
	}
	for(int i =0; i<GridSize; i++){
		if(i==GridSize-1)
			cout<<"0      ";
		else if(i == GridSize-1-12)
			cout<<"50000  ";
		else if(i == 0)
			cout<<"100000 ";
		else
		cout<<"       ";
		for(int j =0; j<GridSize; j++){
			cout<<xygrid[i][j]<<" ";
		}
		cout<<endl;
	}
	cout<<right<<setw(33)<<"50000"<<setw(28)<<"100000"<<endl;
	for(const auto& ad : RadarData){
	cout << "Flight ID: " << ad.id << endl;
	cout << "Flight Position: (" << ad.x << ", " << ad.y << ", " << ad.z << ")" << endl;
	}

	//X-Z plane
	cout<<"X-Z plan view"<<endl;
	char xzgrid[GridSize][GridSize];
	for(int i =0; i<GridSize; i++){
			for(int j =0; j<GridSize; j++){
				xzgrid[i][j]='-';
			}
		}
		for(const auto& ad : RadarData){
			int xPos = static_cast<int>(ad.x/100000 *(GridSize-1));
			int zPos = static_cast<int>((ad.z-15000)/250000 *(GridSize-1));
			if(xPos >=0 && xPos < GridSize && zPos >=0 && zPos < GridSize){
				xzgrid[GridSize-1-zPos][xPos]='0' + ad.id;
			}
		}
		for(int i =0; i<GridSize; i++){
			if(i==GridSize-1)
						cout<<"15000  ";
					else if(i == GridSize-1-12)
						cout<<"50000  ";
					else if(i == 0)
						cout<<"100000 ";
					else
					cout<<"       ";
					for(int j =0; j<GridSize; j++){
						cout<<xygrid[i][j]<<" ";
					}
					cout<<endl;
				}
				cout<<right<<setw(33)<<"50000"<<setw(28)<<"100000"<<endl;
			for(const auto& ad : RadarData){
			cout << "Flight ID: " << ad.id << endl;
			cout << "Flight Position: (" << ad.x << ", " << ad.y << ", " << ad.z << ")" << endl;
			}
	pthread_mutex_unlock(&print_mutex);
}
double Radar::CalculateDistance(const AircraftData& data) const {
	return sqrt(data.x*data.x + data.y*data.y + data.z*data.z);
}

double Radar::CalculateAltitude(const AircraftData& data) const{
	return data.z *3.2808399;//meter to feet
}

double Radar::CalculateSpeed(const AircraftData& data) const {
	return sqrt(data.speedX*data.speedX + data.speedY*data.speedY + data.speedZ*data.speedZ);
}

void Radar::InitializeSemaphore() {
    const char* SEM_NAME = "/aircraft_sem";
    shm_sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (shm_sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }
}

void Radar::DestroySemaphore() {
    if (shm_sem != SEM_FAILED) {
        sem_close(shm_sem);
    }
}
void Radar::TimerHandler(union sigval sv) {
    Radar* radar = static_cast<Radar*>(sv.sival_ptr);
    radar->ReadData();
    radar->print();
}
void Radar::StartTimer() {
    struct sigevent sev;
    struct itimerspec its;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = TimerHandler;
    sev.sigev_value.sival_ptr = this;
    sev.sigev_notify_attributes = nullptr;

    if (timer_create(CLOCK_REALTIME, &sev, &timer_id) == -1) {
        cerr << "Error creating timer: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    its.it_value.tv_sec = 1;    // First update after 1 second
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 3; // Repeat every 3 seconds
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timer_id, 0, &its, nullptr) == -1) {
        cerr << "Error setting timer: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
}

void* RadarThread(void* arg) {
    Radar* radar = static_cast<Radar*>(arg);
    radar->StartTimer();
    while(1){sleep(1);}
    return nullptr;
}

int main() {
	Radar::InitializeSemaphore();
    Radar radar;
    pthread_t thread;

    if (pthread_create(&thread, nullptr, RadarThread, &radar) != 0) {
        cerr << "Error creating thread" << endl;
        return 1;
    }

    pthread_join(thread, nullptr);
    Radar::DestroySemaphore();
    return 0;
}
