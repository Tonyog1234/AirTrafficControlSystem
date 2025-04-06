#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <sys/dispatch.h>
using namespace std;
const int MAX_AIRCRAFT = 6;
const int SHM_SIZE = 4096;

struct AircraftData {
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    bool status;
};
typedef struct {
    unsigned int id;
    char body[500];
} msg_struct;

class Aircraft {
private:
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    bool status = true;

    void* shm_ptr;        // Pointer to shared memory (passed from main)
    size_t shm_offset;    // Offset in shared memory for this aircraft
    timer_t timer_id;     // Timer ID

    static sem_t* shm_sem;  // Shared semaphore for memory access
    static pthread_mutex_t print_mutex;// only one mutex for all aircraft
public:
    Aircraft(int id, double x, double y, double z, double speedX, double speedY, double speedZ, void* shm, size_t offset);
    ~Aircraft();

    int getID() const;
    double getX() const;
    double getY() const;
    double getZ() const;
    double getSpeedX() const;
    double getSpeedY() const;
    double getSpeedZ() const;
    bool getStatus() const;

    void setID(int id);
    void setX(double x);
    void setY(double y);
    void setZ(double z);
    void setSpeedX(double speedX);
    void setSpeedY(double speedY);
    void setSpeedZ(double speedZ);
    void setStatus(bool status);

    void UpdatePosition();
    void CheckAirSpace();
    void print();
    void UpdateShareMemory();

    static void InitializeSemaphore();  // To set up semaphore
    static void DestroySemaphore();     // To clean up
    void logAirspace(void* shm_ptr);
    static void TimerHandler(union sigval sv);
    void StartTimer();
    void StartServer(vector<Aircraft>& aircrafts);

};

#endif // AIRCRAFT_H
