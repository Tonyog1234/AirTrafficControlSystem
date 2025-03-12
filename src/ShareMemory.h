#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <semaphore.h>

// Constants for airspace dimensions
const int AIRSPACE_WIDTH = 100000;
const int AIRSPACE_HEIGHT = 25000;
const int AIRSPACE_LENGTH = 100000;
const int MIN_SEPARATION_XY = 3000;
const int MIN_SEPARATION_Z = 1000;
const int MAX_AIRCRAFT = 5;  // Max aircraft in airspace

// Aircraft Data Structure
class AircraftData {
private:
    int id;
    double x, y, z;         // Position
    double speedX, speedY, speedZ; // Speed in each direction
    bool status=true;
    AircraftData* ShareAircraft;
    sem_t semaphore;
public:
    AircraftData();
    AircraftData(int id, double x, double y, double z, double speedX, double speedY, double speedZ);
    ~AircraftData();

    //Getter
       int getID() const;

       double getX() const;
       double getY() const;
       double getZ() const;

       double getSpeedX() const;
       double getSpeedY() const;
       double getSpeedZ() const;

       bool getStatus() const;

       AircraftData* getPtr() const;


       //Setter
       void setID(int id) ;

       void setX(double) ;
       void setY(double) ;
       void setZ(double) ;

       void setSpeedX(double) ;
       void setSpeedY(double) ;
       void setSpeedZ(double) ;

       void setStatus(bool);

       void setPtr(AircraftData*);


       //Store
       void Store(int id, double x, double y, double z, double speedX, double speedY, double speedZ);

       //Print
       void print();

};

#endif // SHARED_MEMORY_H
