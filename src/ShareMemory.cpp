#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include "ShareMemory.h"
using namespace std;

AircraftData::AircraftData(){

		id=0;
		x=0;
		y=0;
		z=0;
		speedX=0;
		speedX=0;
		speedX=0;
		status=true;
		ShareAircraft = new AircraftData[MAX_AIRCRAFT];
}
    //Constructor
AircraftData::AircraftData(int id, double x, double y, double z, double speedX, double speedY, double speedZ){
			this->id=id;
			this->x=x;
			this->y=y;
			this->z=z;
			this->speedX=speedX;
			this->speedY=speedY;
			this->speedZ=speedZ;
			status=false;
			ShareAircraft = new AircraftData[MAX_AIRCRAFT];
 }

    //Getter
int AircraftData::getID() const{
	return id;

}

double AircraftData::getX() const{
	return x;
}
double AircraftData::getY() const{
	return y;
}
double AircraftData::getZ() const{
	return z;
}

double AircraftData::getSpeedX() const{
	return speedX;
}
double AircraftData::getSpeedY() const{
	return speedY;
}
double AircraftData::getSpeedZ() const{
	return speedZ;
}
bool AircraftData::getStatus() const{
	return status;
}
AircraftData* AircraftData::getPtr() const{
	return ShareAircraft;
}
    //Setter
void AircraftData::setID(int id) {
	this->id=id;
}

void AircraftData::setX(double x) {
	this->x=x;
}
void AircraftData::setY(double y) {
	this->y=y;
}
void AircraftData::setZ(double z) {
	this->z=z;
}

void AircraftData::setSpeedX(double speedX) {
	this->speedX=speedX;
}
void AircraftData::setSpeedY(double speedY) {
	this->speedY=speedY;
}
void AircraftData::setSpeedZ(double speedZ) {
	this->speedZ=speedZ;
}
void AircraftData::setStatus(bool status){
	this->status=status;
}
void AircraftData::setPtr(AircraftData* ptr){
	ShareAircraft=ptr;
}
    //Print
void AircraftData::print(){
	cout<<"******************************"<<endl;
	cout<<"This is share memory data"<<endl;
	cout<<"Flight ID: "<< id <<endl;
	cout<<"Flight Position: ("<<x<<", "<<y<<", "<<z<<")"<<endl;
	cout<<"Flight Speed: ("<<speedX<<", "<<speedY<<", "<<speedZ<<")"<<endl;
	cout<<"******************************"<<endl;
}
//Store
void AircraftData::Store(int id, double x, double y, double z, double speedX, double speedY, double speedZ){
		setID(id);
		setX(x);
		setY(y);
		setZ(z);
		setSpeedX(speedX);
		setSpeedY(speedY);
		setSpeedY(speedZ);
}
AircraftData::~AircraftData(){
	delete ShareAircraft;
}

