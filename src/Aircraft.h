#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include "ShareMemory.h"

class Aircraft {
private:
    int id;
    double x, y, z;
    double speedX, speedY, speedZ;
    bool active;
   // SharedAirspace* airspace;
public:
    //Default constructor
    Aircraft();

    //Constructor
    Aircraft(int id, double x, double y, double z, double speedX, double speedY, double speedZ, bool active);

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
    void UpdatePosition(double x, double y, double z, double speedX, double speedY, double speedZ, int time);

    //Print
    void print();
};
#endif // AIRCRAFT_H
