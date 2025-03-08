#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include<fstream>
#include "Aircraft.h"
using namespace std;

Aircraft::Aircraft(){
		ifstream input;
		input.open("input.txt");
		//exception handling
		try{
			if(!input){
				cout<<"Input file cannot be opened"<<endl;
				exit(0);
			}
			else{
				input>>id>>x>>y>>z>>speedX>>speedY>>speedZ;
				setID(id);
				setX(x);
				setY(y);
				setZ(z);
				setSpeedX(speedX);
				setSpeedY(speedY);
				setSpeedZ(speedZ);
			}
		}
		catch(...){
					cout<<"Exception! Check Input file"<<endl;
		}

		input.close();

		if ( (x < 0 || x > 100000) || (y < 0 || y > 100000) || (z < 15000 || z > 40000) )
		{
			            status = false; // Exit airspace
		}

}
    //Constructor
Aircraft::Aircraft(int id, double x, double y, double z, double speedX, double speedY, double speedZ){
			this->id=id;
			this->x=x;
			this->y=y;
			this->z=z;
			this->speedX=speedX;
			this->speedY=speedY;
			this->speedZ=speedZ;
			status=false;

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
	return status;
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
	this->status=status;
}

    //Update
void Aircraft::UpdatePosition(double x, double y, double z, double speedX, double speedY, double speedZ){
	this->x+= speedX;
	this->y+= speedY;
	this->z+= speedZ;
	if (x < 0 || x > 100000 || y < 0 || y > 100000 || z < 15000 || z > 40000) {
	            status = false; // Exit airspace
	        }
	print();
}

    //Print
void Aircraft::print(){
	cout<<"Flight ID: "<< id <<endl;
	cout<<"Flight Position: ("<<x<<", "<<y<<", "<<z<<")"<<endl;
	cout<<"Flight Speed: ("<<speedX<<", "<<speedY<<", "<<speedZ<<")"<<endl;
}


