#ifndef RADAR_H
#define RADAR_H

#include <vector>
#include <semaphore.h>
#include <pthread.h>
using namespace std;
const int MAX_AIRCRAFT = 5;
const int SHM_SIZE = 4096;

struct AircraftData {
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    bool status;
};

class Radar {
private:
    vector<AircraftData> RadarData;
    int shm_fd = -1;      // File descriptor for shared memory
    void* shm_ptr = nullptr;  // Pointer to mapped shared memory
    timer_t timer_id;     // Timer ID for periodic updates
    int range= 146970;// Max range

    static sem_t* shm_sem;
    static pthread_mutex_t print_mutex;
public:
    Radar();
    ~Radar();
    void ReadData();
    void print();
    double CalculateDistance(const AircraftData&) const;
    double CalculateAltitude(const AircraftData&) const;
    double CalculateSpeed(const AircraftData&) const;
    static void TimerHandler(union sigval sv);
    void StartTimer();

    static void InitializeSemaphore();
    static void DestroySemaphore();
};

#endif
