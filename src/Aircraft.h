#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <vector>
using namespace std;
const int MAX_AIRCRAFT = 5;
const int SHM_SIZE = 4096;

// Struct for shared memory data
struct AircraftData {
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    bool status;
};

class Aircraft {
private:
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    bool status= true;
    vector<Aircraft> airplane;
    vector<AircraftData> data;

    // Shared memory members
    int shm_fd=-1;          // File descriptor for shared memory
    void* shm_ptr=nullptr;       // Pointer to mapped shared memory
public:
    //Default constructor
    Aircraft();

    //Constructor
    Aircraft(int id, double x, double y, double z, double speedX, double speedY, double speedZ);

    ~Aircraft();
    //Getter
    int getID() const;

    double getX() const;
    double getY() const;
    double getZ() const;

    double getSpeedX() const;
    double getSpeedY() const;
    double getSpeedZ() const;

    bool getStatus() const;

    //Setter
    void setID(int id) ;

    void setX(double) ;
    void setY(double) ;
    void setZ(double) ;

    void setSpeedX(double) ;
    void setSpeedY(double) ;
    void setSpeedZ(double) ;

    void setStatus(bool);

    //Update
    void UpdatePosition();
    void CheckAirSpace();

    //Print
    void print();

    //Functions for Timer
    static void TimerHandler(union sigval sv);
    void  StartTimer();

};
#endif // AIRCRAFT_H
