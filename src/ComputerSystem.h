#ifndef COMPUTERSYSTEM_H_
#define COMPUTERSYSTEM_H_

#include <vector>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>  // For QNX message passing
#include <sys/netmgr.h>    // For name_open
using namespace std;

const int MAX_AIRCRAFT = 6;
const int SHM_SIZE = 4096;

static const double MIN_HORIZONTAL_SEPARATION = 3000.0;
static const double MIN_VERTICAL_SEPARATION   = 1000.0;

static const double MIN_X = 0.0;
static const double MAX_X = 100000.0;
static const double MIN_Y = 0.0;
static const double MAX_Y = 100000.0;
static const double MIN_Z = 15000.0;
static const double MAX_Z = 40000.0;

typedef struct {
    unsigned int id;
    char body[100];
} msg_struct;

struct AircraftData {
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    bool status;

};

class ComputerSystem {
private:
    int    shm_fd     = -1;
    void*  shm_ptr    = nullptr;
    sem_t* shm_sem    = nullptr;
    timer_t timer_id  = 0;
    bool alert=false;
    vector<AircraftData> aircraftList;

public:
    ComputerSystem();
    ~ComputerSystem();

    bool getAlert();

    void ReadData();
    void CheckForAlerts();
    void print();
    void RequestCommand();
    void StartTimer();
    static void TimerHandler(union sigval sv);

private:
    void OpenSharedMemory();
    void CloseSharedMemory();
    void InitializeSemaphore();
    void DestroySemaphore();
};




#endif /* COMPUTERSYSTEM_H_ */
