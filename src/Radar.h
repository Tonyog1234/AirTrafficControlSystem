#ifndef RADAR_H
#define RADAR_H

#include <vector>
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

public:
    Radar();
    ~Radar();
    void ReadData();
    void print();

    static void TimerHandler(union sigval sv);
    void StartTimer();
};

#endif
