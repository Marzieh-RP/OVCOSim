#ifndef globals_h
#define globals_h

#include "computeResource.h"
#include "workflowgraphs.h"
#include <vector>
using namespace std;

enum algorithms {bruteforce, randomized, rand_hybrid, ga, rl_hybrid};//supported assignment algorithms
algorithms algorithm = randomized;//default algorithm
int algorithm_iterations = 100;//default number of iterations
bool usegnuplot = false;//by default, gnuplot is not used
bool usegraphviz = false;//by default, graphviz is not used
bool dvfs = false;//by default

/*values used in the cost function*/
double time_spent;
double energy;
double cost;
double time_weight;
double energy_weight;
double cost_weight;

//used to visualize progression of cost function
double best_time, best_cost, best_pow;

int print_helper;

//not a user feature, just to help me draw how the cost function changes with different assignments
bool dump_more_stats = false;



//success rates for algorithms
vector<int> total_bf;
vector<double> success_bf;
int selected_bit;
int max_bf_size = 6;

//global objects of links, resources, and workflow graphs
vector<vector<link>> links;
vector<computeResource> resources;
vector<wfg> wfgs;

#endif
