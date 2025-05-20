#ifndef COMPUTER_SYSTEM_H
#define COMPUTER_SYSTEM_H

#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/dispatch.h>
#include <set>
using namespace std;

const int MAX_AIRCRAFT = 10;
const int SHM_SIZE = 4096;
const double MIN_X = 0, MAX_X = 100000;
const double MIN_Y = 0, MAX_Y = 100000;
const double MIN_Z = 15000, MAX_Z = 40000;
const double MIN_HORIZONTAL_SEPARATION = 3000;
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
    set<int> alertedAircraft;
    int shm_fd = -1;
    int index=0; // index for aircraft
    int CollisionCount=0;
    int BoundCount=0;
    int CollisionIndex;
    int onetimeBound=1;
    int onetimeCollision=1;
    static sem_t* shm_sem;
    bool AlertOutofBound = false;
    bool AlertCollision=false;
    timer_t timer_id = 0;
    name_attach_t* attach = nullptr;
    void* shm_ptr = nullptr;

public:
    ComputerSystem();
    ~ComputerSystem();
    bool getAlertOutofBound();
    bool getAlertCollision();
    int getOneTimeBound();
    int getOneTimeCollision();

    int getCollisionCount();
    void setCollisionCount();
    void IncrementCollisionCount();

    int getBoundCount();
    void setBoundCount();
    void IncrementBoundCount();

    void setOneTimeCollision();
    void setOneTimebound();
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
