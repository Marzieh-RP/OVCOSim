/* computeResource class definition */
#ifndef comp_h
#define comp_h
#include <string>
using namespace std;

class computeResource{
private:
    string name;//name of resource
    double power;//power of resource, only applicable to local device since we care about energy
    double idlepower;//idle power of resource, only applicable to local device
    double speed;//computation speed of resource
    double cost_per_second;//cost to use the resource per second
    bool is_busy;//is the resource busy running a task?
public:
    computeResource();//default constructor
    /*accessor functions*/
    double getSpeed();
    double getCost();
    double getBusy();
    double getFree();
    string getName();
    bool checkBusy();
    double getPow();
    double getIdlePow();
    /*mutator functions*/
    void setName(string n);
    void setPow(double d);
    void setIdlePow(double d);
    void setSpeed(double s);
    void setCost(double c);
};

class link{
private:
    string name;//name of link
    double speed;//speed of link, in MBps
    double latency;//latency of link, in ms
    bool is_busy;//is the link busy?
public:
    void initialize(string n, double s, double l);//initialize the link
    /*accessor functions*/
    double getSpeed();
    double getLatency();
    bool checkBusy();
    string getName();
    /*mutators for is_busy*/
    void getBusy();
    void getFree();
};



#endif
