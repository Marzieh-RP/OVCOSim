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
    cores_used = 0;
    cores = 1;
    cost_per_second = 0;
    devtype = 0;
}
/*accessors for compute Resource*/

double computeResource::getSpeed(){
    return speed;
}
int computeResource::getType(){
    return devtype;
}
int computeResource::getCores(){
    return cores;
}

void computeResource::setCores(int c){
    cores = c;
}
void computeResource::setType(string mytype){
    if (mytype == "mobile")
        devtype = 1;
    else
        devtype = 0;
}
double computeResource::getCost(){
    return cost_per_second;
}
void computeResource::getBusy(){
    cores_used++;
    if (cores_used == cores)
        is_busy = true;
}
void computeResource::getFree(){
    
    if (cores_used > 0)
        cores_used--;
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

vector<pair<double,double> > computeResource::getspratios(){

    return spratios;
}


/*mutators for compute Resource*/

void computeResource::setCost(double c){
    cost_per_second = c;
}

void computeResource::addPS(double s, double p){

    spratios.push_back(make_pair(s,p));

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




