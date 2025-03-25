#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include <fstream>
#include <csignal>
#include <ctime>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <pthread.h>
#include "Aircraft.h"
using namespace std;

//Need to define mutex in cpp file
pthread_mutex_t Aircraft::print_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t* Aircraft::shm_sem = nullptr;

Aircraft::Aircraft(int id, double x, double y, double z, double speedX, double speedY, double speedZ, void* shm, size_t offset)
    : id(id), x(x), y(y), z(z), speedX(speedX), speedY(speedY), speedZ(speedZ), shm_ptr(shm), shm_offset(offset), timer_id(0) {
    if (offset == sizeof(int)) {
        *(static_cast<int*>(shm_ptr)) = 0;//First Aircraft arrives, 0 object in the share memory
    }
    UpdateShareMemory();
    print();
}

Aircraft::~Aircraft() {
    if (timer_id != 0) {
        if (timer_delete(timer_id) == -1) {
            cerr << "Error deleting timer for Flight ID " << id << ": " << strerror(errno) << endl;
        } else {
            cout << "Timer deleted for Flight ID " << id << endl;
            timer_id = 0;  // Prevent double deletion
        }
    }
}

int Aircraft::getID() const { return id; }
double Aircraft::getX() const { return x; }
double Aircraft::getY() const { return y; }
double Aircraft::getZ() const { return z; }
double Aircraft::getSpeedX() const { return speedX; }
double Aircraft::getSpeedY() const { return speedY; }
double Aircraft::getSpeedZ() const { return speedZ; }
bool Aircraft::getStatus() const { return status; }

void Aircraft::setID(int id) { this->id = id; }
void Aircraft::setX(double x) { this->x = x; }
void Aircraft::setY(double y) { this->y = y; }
void Aircraft::setZ(double z) { this->z = z; }
void Aircraft::setSpeedX(double speedX) { this->speedX = speedX; }
void Aircraft::setSpeedY(double speedY) { this->speedY = speedY; }
void Aircraft::setSpeedZ(double speedZ) { this->speedZ = speedZ; }
void Aircraft::setStatus(bool status) { this->status = status; }

void Aircraft::UpdatePosition() {
    x += speedX;
    y += speedY;
    z += speedZ;
    CheckAirSpace();
    UpdateShareMemory();
    print();
}

void Aircraft::UpdateShareMemory() {
	sem_wait(shm_sem);
    AircraftData ad = {id, x, y, z, speedX, speedY, speedZ, status};
    memcpy(static_cast<char*>(shm_ptr) + shm_offset, &ad, sizeof(AircraftData));
    if (shm_offset == sizeof(int)) {
        *(static_cast<int*>(shm_ptr)) = MAX_AIRCRAFT;//update number of object in share memory for other process
    }
    sem_post(shm_sem);
}

void Aircraft::CheckAirSpace() {
    if (x < 0 || x > 100000 || y < 0 || y > 100000 || z < 15000 || z > 40000) {
        status = false;
    }
}

void Aircraft::print() {
	pthread_mutex_lock(&print_mutex);
    cout << "Flight ID: " << id << endl;
    cout << "Flight Position: (" << x << ", " << y << ", " << z << ")" << endl;
    cout << "Flight Speed: (" << speedX << ", " << speedY << ", " << speedZ << ")" << endl;
    cout << "Flight Status: " << status << endl;
    cout << "****************************" << endl;
    pthread_mutex_unlock(&print_mutex);
}
void Aircraft::InitializeSemaphore(){
	const char* SEM_NAME = "/aircraft_sem";
		    shm_sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0666, 1);  // Initial value 1 (unlocked)
		    if (shm_sem == SEM_FAILED) {
		        if (errno == EEXIST) {
		            shm_sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
		        } else {
		            perror("sem_open failed");
		            exit(1);
		        }
		    }
}
void Aircraft::DestroySemaphore(){
	if (shm_sem != SEM_FAILED) {
	        sem_close(shm_sem);
	        sem_unlink("/aircraft_sem");
	    }
}
void Aircraft::TimerHandler(union sigval sv) {
    Aircraft* aircraft = static_cast<Aircraft*>(sv.sival_ptr);
    aircraft->UpdatePosition();
}

void Aircraft::StartTimer() {
    struct sigevent sev;
    struct itimerspec its;

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_notify_function = TimerHandler;
    sev.sigev_value.sival_ptr = this;
    sev.sigev_notify_attributes = nullptr;

    if (timer_create(CLOCK_REALTIME, &sev, &timer_id) == -1) {
        cerr << "Error creating timer for Flight ID " << id << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    its.it_value.tv_sec = 1;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 3;
    its.it_interval.tv_nsec = 0;

    if (timer_settime(timer_id, 0, &its, nullptr) == -1) {
        cerr << "Error setting timer for Flight ID " << id << ": " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }
}

void* AircraftThread(void* arg) {
    Aircraft* aircraft = static_cast<Aircraft*>(arg);
    aircraft->StartTimer();
    while (true) { sleep(1); }
    return nullptr;
}

int main() {
	Aircraft::InitializeSemaphore();
    const char* SHM_NAME = "/aircraft_shm";

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
    	perror("shm_open failed"); return 1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
    	perror("ftruncate failed"); return 1;
    }

    void* shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
    	perror("mmap failed"); return 1;
    }

    vector<Aircraft> aircrafts;
    aircrafts.reserve(MAX_AIRCRAFT);  // Prevent reallocation
    vector<pthread_t> threads(MAX_AIRCRAFT);

    ifstream input("input.txt");
    if (!input) {
    	perror("Error opening input.txt"); return 1;
    }
    double x, y, z, speedX, speedY, speedZ;
    int id;
    size_t index = 0;

    while (input >> id >> x >> y >> z >> speedX >> speedY >> speedZ && index < MAX_AIRCRAFT) {

        size_t offset = sizeof(int) + index * sizeof(AircraftData);
        aircrafts.emplace_back(id, x, y, z, speedX, speedY, speedZ, shm_ptr, offset);//need to look it up

        if (pthread_create(&threads[index], nullptr, AircraftThread, &aircrafts[index]) != 0) {
            cerr << "Error creating thread for Aircraft " << id << endl;
            return 1;
        }
        index++;
    }
    input.close();

    //To stop all thread here
    for (size_t i = 0; i < index; i++) {
        pthread_join(threads[i], nullptr);
    }

    //only when main thread exit, then the following will be called.
    munmap(shm_ptr, SHM_SIZE);
    close(shm_fd);
    Aircraft::DestroySemaphore();
    shm_unlink(SHM_NAME);
    return 0;
}
