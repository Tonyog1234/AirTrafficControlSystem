#ifndef COMPUTER_SYSTEM_H
#define COMPUTER_SYSTEM_H

#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/dispatch.h>
using namespace std;

const int MAX_AIRCRAFT = 6;
const int SHM_SIZE = 4096;
const double MIN_X = 0, MAX_X = 100000;
const double MIN_Y = 0, MAX_Y = 100000;
const double MIN_Z = 15000, MAX_Z = 40000;
const double MIN_HORIZONTAL_SEPARATION = 5000;
const double MIN_VERTICAL_SEPARATION = 1000;

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

class ComputerSystem {
private:
    vector<AircraftData> aircraftList;
    int shm_fd = -1;
    void* shm_ptr = nullptr;
    int index=0; // index for aircraft
    int CollisionIndex;
    int onetimeBound=2;
    int onetimeCollision=2;
    static sem_t* shm_sem;
    bool AlertOutofBound = false;
    bool AlertCollision=false;
    timer_t timer_id = 0;
    name_attach_t* attach = nullptr;


public:
    ComputerSystem();
    ~ComputerSystem();
    bool getAlertOutofBound();
    bool getAlertCollision();
    int getOneTimeBound();
    int getOneTimeCollision();
    void OpenSharedMemory();
    void CloseSharedMemory();
    void InitializeSemaphore();
    void DestroySemaphore();
    void ReadData();
    void OutofBoundAlerts();
    void CollisionAlerts();
    void print();
    void RequestCommand();
    void SolveCollision();
    static void TimerHandler(union sigval sv);
    void StartTimer();
    void StartInfoServer();
    static void* InfoServerThread(void* arg);  // Declare as static member
};

#endif
