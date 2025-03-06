#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include "Aircraft.h"
using namespace std;

Aircraft::Aircraft(){
		id=0;
		x=0;
		y=0;
		z=0;
		speedX=0;
		speedY=0;
		speedZ=0;
		active=false;
}
    //Constructor
Aircraft::Aircraft(int id, double x, double y, double z, double speedX, double speedY, double speedZ, bool status){
			this->id=id;
			this->x=x;
			this->y=y;
			this->z=z;
			this->speedX=speedX;
			this->speedY=speedY;
			this->speedZ=speedZ;
			this->active=status;
 }

    //Getter
int Aircraft::getID() const{
	return id;

}

double Aircraft::getX() const{
	return x;
}
double Aircraft::getY() const{
	return y;
}
double Aircraft::getZ() const{
	return z;
}

double Aircraft::getSpeedX() const{
	return speedX;
}
double Aircraft::getSpeedY() const{
	return speedY;
}
double Aircraft::getSpeedZ() const{
	return speedZ;
}
bool Aircraft::getStatus() const{
	return active;
}

    //Setter
void Aircraft::setID(int id) {
	this->id=id;
}

void Aircraft::setX(double x) {
	this->x=x;
}
void Aircraft::setY(double y) {
	this->y=y;
}
void Aircraft::setZ(double z) {
	this->z=z;
}

void Aircraft::setSpeedX(double speedX) {
	this->speedX=speedX;
}
void Aircraft::setSpeedY(double speedY) {
	this->speedX=speedY;
}
void Aircraft::setSpeedZ(double speedZ) {
	this->speedX=speedZ;
}
void Aircraft::setStatus(bool status){
	active=status;
}

    //Update
void Aircraft::UpdatePosition(double x, double y, double z, double speedX, double speedY, double speedZ, int time){
	this->x+= speedX*time;
	this->y+= speedY*time;
	this->z+= speedZ*time;
	if (x < 0 || x > 100000 || y < 0 || y > 100000 || z < 0 || z > 25000) {
	            active = false; // Exit airspace
	        }
}

    //Print
void Aircraft::print(){
	cout<<"Flight ID: "<< id <<endl;
	cout<<"Flight Position: ("<<x<<", "<<y<<", "<<z<<")"<<endl;
	cout<<"Flight Speed: ("<<speedX<<", "<<speedY<<", "<<speedZ<<")"<<endl;
}


