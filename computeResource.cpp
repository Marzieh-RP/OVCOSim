/* computeResource class definition */
#include <string>
#include "computeResource.h" 
using namespace std;

//default constructor for compute resource
computeResource::computeResource(){
    name = "";
    power = 0;
    idlepower = 0;
    speed = 0;
    is_busy = false;
    cost_per_second = 0;
}
/*accessors for compute Resource*/

double computeResource::getSpeed(){
    return speed;
}
double computeResource::getCost(){
    return cost_per_second;
}
double computeResource::getBusy(){
    is_busy = true;
}
double computeResource::getFree(){
    is_busy = false;
}
double computeResource::getPow(){
    return power;
}
double computeResource::getIdlePow(){
    return idlepower;
}
bool computeResource::checkBusy(){
    return is_busy;
}
string computeResource::getName(){
    return name;
}
/*mutators for compute Resource*/

void computeResource::setCost(double c){
    cost_per_second = c;
}

void computeResource::setName(string n){
    name = n;
}
void computeResource::setPow(double d){
    power = d;
}
void computeResource::setIdlePow(double d){
    idlepower = d;
}

void computeResource::setSpeed(double s){
    speed = s;
}

//initialization for link
void link::initialize(string n, double s, double l){
    name = n;
    speed = s;
    latency = l;
    is_busy = false;
}

/*mutators for link*/
double link::getSpeed(){
    return speed;
}
double link::getLatency(){
    return latency;
}
bool link::checkBusy(){
    return is_busy;
}
string link::getName(){
    return name;
}
/*accessors for link*/
void link::getBusy(){
    is_busy = true;
}
void link::getFree(){
    is_busy = false;
}




