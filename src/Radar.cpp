#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <csignal>
#include <pthread.h>
#include "Radar.h"
using namespace std;

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

    // Read the number of aircraft from shared memory
    int count = *(static_cast<int*>(shm_ptr));
    if (count < 0 || count > MAX_AIRCRAFT) {
        cerr << "Invalid aircraft count: " << count << endl;
        return;
    }

    // Read AircraftData array from shared memory
    AircraftData* data = reinterpret_cast<AircraftData*>(static_cast<char*>(shm_ptr) + sizeof(int));
    for (int i = 0; i < count; i++) {
        RadarData.push_back(data[i]);
    }
}

void Radar::print() {
    cout << "Radar Data:" << endl;
    for (const auto& ad : RadarData) {
        cout << "Flight ID: " << ad.id << endl;
        cout << "Flight Position: (" << ad.x << ", " << ad.y << ", " << ad.z << ")" << endl;
        cout << "Flight Speed: (" << ad.speedX << ", " << ad.speedY << ", " << ad.speedZ << ")" << endl;
        cout << "Flight Status: " << ad.status << endl;
        cout << "****************************" << endl;
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
    return nullptr;
}

int main() {
    Radar radar;
    pthread_t thread;

    if (pthread_create(&thread, nullptr, RadarThread, &radar) != 0) {
        cerr << "Error creating thread" << endl;
        return 1;
    }

    pthread_join(thread, nullptr);
    return 0;
}
