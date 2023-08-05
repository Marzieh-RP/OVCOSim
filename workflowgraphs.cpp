#include <vector>
#include <limits>
#include <string>
#include <iostream>
#include "workflowgraphs.h"
#include <random>
#include <unordered_set>

using namespace std;
//default constructor
wfg::wfg(){ 
    all_done =false;
}
//consructor that also sets name of the graph
wfg::wfg(string n){
    name = n;
    all_done = false;
}
//initialize the matrix
void wfg::initialize_matrix(){ 
    vector<double> weights(num_nodes,-1);
    for (int i=0;i<num_nodes;i++){
        adj_matrix.push_back(weights);
    }
}

/*mutators*/
//set the number of computation resources
void wfg::setNumComp(int n){
    num_compute_resources = n;
}
//set the name of the graph
void wfg::setName(string n){
    name = n;
}
//set number of nodes
void wfg::setNumNodes(int n){
    num_nodes = n;
}
/*accesors*/
//get the value of the matrix from s to d
double wfg::get_matrix(int s, int d){
    return adj_matrix[s][d];
}

/*main functions*/
//this function transforms the graph such that communication tasks are created
void wfg::transform_graph(){
    //for every compute task, there is a task
    for (int i=0;i<num_nodes;i++){
        task temp;
        temp.type = 0;//type 0 means computation task
        temp.assigned_to = assignments[i]; //assignment needs to be known
        temp.load = processing_time[i]/compute_speed[assignments[i]]; //processing time divided by compute speed of the assigned resource shows time it takes to run the task
        tasks.push_back(temp);//add it to list of tasks
    }
    //see if subsequent tasks are on the same resource or not.
    //if not, the weight turns into a separate communication task
    for (int i=0;i<num_nodes;i++){
        for (int j=0;j<num_nodes;j++){
            if (i==j) continue;//avoid same index
            if (adj_matrix[i][j] == -1)//if there's no connection between the two tasks, go to the next one
                continue;
            if (assignments[i] == assignments[j])//if they are assigned to the same resource, go to the next one
                continue;
            task temp;
            temp.type = 1;//communication task
            temp.assigned_to = num_compute_resources * assignments[i] + assignments[j] + num_compute_resources;// make a new number from i and j. this is tricky, we need to be able to get i and j back from this number.
            temp.load = (adj_matrix[i][j]*8)/link_speeds[assignments[i]][assignments[j]];//communication load converted to bits divided by speed of link
            temp.load += link_latencies[assignments[i]][assignments[j]];//plus latency between the two resources
            if (temp.load != 0)//realistically if there's any latency, it should never be zero, but to avoid crashes, this needs to be checked
                tasks.push_back(temp);//push only non-zero loads
        }
    }
    //dependecy relations between all tasks
    vector<bool> temp(tasks.size(),false);
    for (int i=0;i<tasks.size();i++)
        waiting_on.push_back(temp);//initialize all tofalse

    int com_task_index = num_nodes;//index of task for communication
    for (int i=0;i<num_nodes;i++){
        for (int j=0;j<num_nodes;j++){
            if (i==j) continue;
            if (adj_matrix[i][j] == -1)
                continue;
            if (assignments[i] == assignments[j] || (adj_matrix[i][j] == 0 && link_latencies[assignments[i]][assignments[j]] == 0))//if no communication task is created
            {
                waiting_on[j][i] = true;//computation tasks
            }
            else {

                waiting_on[j][com_task_index] = true;//task j is waiting on the comunication task
                waiting_on[com_task_index][i] = true;//communication task is waiting on task i
                com_task_index++;//update the communication task index
            }
        }
    }

    tasks_ready_to_run.clear();//probably not needed, but just to make sure I didn't change it elsewhere in the code, clear it
    tasks_ran.clear();//same as bove
    tasks_running.clear();//same as above
    for (int i=0;i<tasks.size();i++)
    {
        tasks_ready_to_run.push_back(false);//no task is ready to run at the start
        tasks_ran.push_back(false);//no task has finished running
        tasks_running.push_back(false);//no task is running
    }
}

//this function returns a vector of strings that is used in visualizing the transformed graph in graphviz
vector<string> wfg::visualize_transformed_matrix(){
    vector<string> result;
    result.push_back("subgraph cluster_"+name+"{");//name of graph
    result.push_back("label=\""+name+"\"");//label of graph
    //push back all tasks
    for (int i=0;i<tasks.size();i++){
        if (tasks[i].type == 0)//computation tasks
            result.push_back("    T"+name+"x"+to_string(i)+" [label=T"+to_string(i)+",shape=\"circle\"]");
        if (tasks[i].type == 1)//communication tasks
            result.push_back("    T"+name+"x"+to_string(i)+" [label=C"+to_string(i-num_nodes)+",shape=\"doublecircle\"]");
    }

    //push back all relationship between tasks, there are no weights this time
    for (int i=0;i<tasks.size();i++){
        for (int j=0;j<tasks.size();j++){
            if (i==j) continue;
            if (waiting_on[i][j]){
                //there's an edge from j to i
                result.push_back("    T"+name+"x"+to_string(j)+" -> T"+name+"x"+to_string(i));
            }
        }
    }
    result.push_back("    }");//for graphviz syntax
    return result;//return result
}



int wfg::getNumNodes(){
    return num_nodes;
}
int wfg::getNumTasks(){
    return tasks.size();
}

void wfg::initializePT(const vector<double>& data){
    processing_time = data;    
}
void wfg::initializeColors(const vector<string>& co){
    colors = co;
}
string wfg::getColorAt(int index){
    return colors[index];
}
void wfg::setColor(string c){
    color = c;
}   
string wfg::getColor(){
    return color;
}   
void wfg::initializeNames(const vector<string>& n){
    names = n;
}

void wfg::initializeAssignments(const vector<int>& assignment){
    for (int i =0; i<assignment.size();i++){
        assignments.push_back(assignment[i]);
    }
}

void wfg::updateAssignments(const vector<int>& assignment, int start_index){
    for (int i=0;i<assignments.size();i++){
        assignments[i] = assignment[start_index+i];
    }
}

int wfg::getSize(){
    return processing_time.size();
}
double wfg::getpt(int i){
    return processing_time[i];
}
string wfg::getNameat(int i){
    return names[i];
}
string wfg::getName(){
    return name;
}
void wfg::setLinkSpeeds(const vector<vector<double> >& speeds){
    link_speeds = speeds;
}
void wfg::setLinkLatencies(const vector<vector<double> >&latencies){
    link_latencies = latencies;
}
void wfg::change_matrix(int s, int d, double load){
    adj_matrix[s][d] = load;
}

void wfg::findRunnableTasks(){
    for (int i=0;i<tasks.size();i++){
        if (tasks_ready_to_run[i] || tasks_ran[i] || tasks_running[i])
            continue;
        bool can_be_run = true;
        for (int j=0;j<tasks.size();j++){
            if (waiting_on[i][j])
                can_be_run = false;
        }
        if (can_be_run)
            tasks_ready_to_run[i] = true;
    }
}

double wfg::findSmallestRunningTask(){
    double smallest = numeric_limits<double>::max();
    for (int i=0;i<tasks.size();i++){
        if (tasks_running[i]){
            if (tasks[i].load < smallest){
                smallest = tasks[i].load;
            }
        }
    }
    return smallest;
}

bool wfg::startRunningTasks(int type, int resource_num){//type =0 for compute, 1 for links
    //allow this WFG to start running tasks on a given compute resource
    //if it does, return true
    for (int i=0;i<tasks.size();i++){
        if (tasks_ready_to_run[i] && tasks[i].type == type && tasks[i].assigned_to == resource_num)
        {
            tasks_ready_to_run[i] = false;
            tasks_running[i] = true;
            event temp;
            temp.type = 0;
            temp.task_num = i;
            temp.time = total_time;
            events.push_back(temp);
            return true;
        }
    }
    return false;
}

void wfg::runTasks(double time){
    total_time += time;
    for (int i=0;i<tasks.size();i++){
        if (tasks_running[i]){
            tasks[i].load -= time;
        }
    }
}
//if a task is finished, we need to free the resource for others to use. the return serves that purpose

vector<int> wfg::checkFinishedTasks(){
    vector<int> freed;//it is possible for more than one task to finish exactly at the same time, so we need to return a vector
    for (int i=0;i<tasks.size();i++){
        if (tasks_running[i] && tasks[i].load <= 0)//conditions for the task being finished
        {
            //mark the task as no longer running
            tasks_running[i] = false;
            //mark the task as ran
            tasks_ran[i] = true;
            //create a new event
            event temp;
            //type 1 means finished
            temp.type = 1;
            //task number
            temp.task_num = i;
            //end time of task
            temp.time = total_time;
            //push this into events
            events.push_back(temp);

            if (tasks[i].type ==0){
                //for computation tasks
                freed.push_back(tasks[i].assigned_to);
            }
            else 
                freed.push_back(tasks[i].assigned_to);//its a link, so we need to extract the actual resource number from this number
            for (int zz =0;zz<tasks.size();zz++){
                waiting_on[zz][i] = false;//make sure to indicate that tasks are not waiting for task i anymore
            }
        }
    }
    return freed;//return list of resources to be freed
}

//checks to see if the graph has finished running all tasks
bool wfg::graphdone(){
    if (all_done)
        return true;
    for (int i=0;i<tasks.size();i++)
        if (tasks_ran[i] == false)
            return false;
    //once we determine it has finished running, we update this variable so we avoid looping again.
    all_done = true;
    return true;
}

//returns all events
vector<event> wfg::returnEvents(){
    return events;
}

//for debug only, prints events
void wfg::printevents(){
    for (int i=0;i<events.size();i++){
        cout<<events[i].type<<" " <<events[i].task_num<<" " <<events[i].time<<endl;
    }

}

// prints stats about the graph, can be used for debugging
void wfg::printstats(){
    for (int i=0;i<tasks.size();i++)
        cout<<tasks_ready_to_run[i];
    cout<<endl;
    for (int i=0;i<tasks.size();i++)
        cout<<tasks_ran[i];
    cout<<endl;
    for (int i=0;i<tasks.size();i++)
        cout<<tasks_running[i];
    cout<<endl;
    // for (int i=0;i<tasks.size();i++)
    //     cout<<tasks[i].load<<endl;

}


//resets total time
void wfg::resetTime(){
    total_time = 0;
}

//sets the speeds of different compute resources
void wfg::setSpeeds(const vector<double>& speeds){
    compute_speed = speeds;
}

//sets the costs to use different compute resources
void wfg::setCosts(const vector<double>& costs){
    compute_cost = costs;
}

//finds the resource that a task is assigned to
int wfg::findResource(int i){
    if (tasks[i].type == 0)
        return tasks[i].assigned_to;
    //in the visualizer, we don't display same links that don't actually exist, so we need to correct for that
    return tasks[i].assigned_to - num_compute_resources- ((tasks[i].assigned_to - num_compute_resources - 1)/(num_compute_resources+1)) - 1; 
}

//finds the start time of a task
double wfg::findStart(int i){
    for (int j=0;j<events.size();j++)
        if (events[j].task_num == i && events[j].type == 0)
            return events[j].time;
    return 0;
}

//finds the end time of a task
double wfg::findEnd(int i){
    for (int j=0;j<events.size();j++)
        if (events[j].task_num == i && events[j].type == 1)
            return events[j].time;
    return 0;

}

//returns total time
double wfg::getTime(){
    return total_time;
}

//returns assignments
vector<int> wfg::getAssignments(){
    return assignments;
}

//this function reset the graph so that it can be run again, possibly with a different assignment
void wfg::reset(){
    tasks_ready_to_run.clear();
    tasks_ran.clear();
    tasks_running.clear();
    waiting_on.clear();
    events.clear();
    total_time = 0;
    all_done = false;
    tasks.clear();
}

//get the events ready for printing, used for debugging
//this function finds the start time and end time of all tasks and puts them in the vector of strings for printing
//it fills it up first with computation tasks and then communication tasks.
vector<string> wfg::visualize_stats(){
    //for every task, we need to show the duration it was running and on what resource
    vector<string> result;
    for (int i=0;i<tasks.size();i++){
        //compute task
        if (tasks[i].type == 0){
            string temp = "#Graph "+ name+" Task " + to_string(i);
            result.push_back(temp);
            double y_index = tasks[i].assigned_to + 0.5;
            double x_start = 0;
            double x_end = 0;
            for (int j=0;j<events.size();j++){
                if (events[j].task_num == i && events[j].type == 0)
                    x_start = events[j].time;
                else if (events[j].task_num == i && events[j].type == 1)
                    x_end = events[j].time;

            }
            temp = to_string(x_start) + " " +to_string(y_index);
            result.push_back(temp);
            temp = to_string(x_end) + " " +to_string(y_index);
            result.push_back(temp);
            result.push_back("");
            result.push_back("");
        }
        else if (tasks[i].type == 1){
            string temp = "#Graph "+ name+" Communication " + to_string(i);
            result.push_back(temp);
            double y_index = tasks[i].assigned_to + 0.5;
            double x_start = 0;
            double x_end = 0;
            for (int j=0;j<events.size();j++){
                if (events[j].task_num == i && events[j].type == 0)
                    x_start = events[j].time;
                else if (events[j].task_num == i && events[j].type == 1)
                    x_end = events[j].time;

            }
            temp = to_string(x_start) + " " +to_string(y_index);
            result.push_back(temp);
            temp = to_string(x_end) + " " +to_string(y_index);
            result.push_back(temp);
            result.push_back("");
            result.push_back("");
        }
    }

    return result;
}


//this function updates total cost as a result of running this workflow graph
//it also updates total time tasks of this graph ran on the local device, for energy measurement purposes
void wfg::update_costs(double& total_time_local, double& total_cost){
    for (int i = 0; i<events.size();i++){//for all events
        if (events[i].type == 0){//only if event type is "start"
            for (int j=i+1;j<events.size();j++){//for the remaining events
                if(events[j].task_num == events[i].task_num){//if these two events belong to the same task
                    double duration = events[j].time - events[i].time;//duration the task was running
                    if (tasks[events[i].task_num].type == 0){//if it's a computation task
                        int rr = tasks[events[i].task_num].assigned_to;//find the resource it is assigned to
                        total_cost += compute_cost[rr]* duration;//cost of task being run
                        if (rr == 0)//only if it is local
                            total_time_local += duration;//increase local device runtime
                    }
                }
            }
        }
    }
}
//constructor for synthethic workflowgraphs
wfg::wfg(string n, double nodes_avg, double nodes_dev, int seed, double load_avg, double load_dev, double communication_avg, double communication_std, int p_degree, string col) : wfg(n){
    mt19937 g1 (seed); 
    std::normal_distribution<double> random_n(nodes_avg,nodes_dev);
    std::normal_distribution<double> random_load(load_avg,load_dev);
    std::normal_distribution<double> random_com(communication_avg,communication_std);
    std::uniform_int_distribution<int> is_link(0,100);
    //randomly determine size of graph based on distribution
    num_nodes = floor(random_n(g1));
    //use the normal distribution to generate load sizes for computation tasks
    for (int i=0;i<num_nodes;i++)
        processing_time.push_back(random_load(g1));
    initialize_matrix();
    color = col;
    for (int i=0;i<num_nodes;i++){
        assignments.push_back(-1);
        colors.push_back(col);
        names.push_back("T"+to_string(i));
    }
    // depending on the degree of parallelism, decide to add links between tasks
    for (int i=0;i<num_nodes;i++)
        for (int j=i+1;j<num_nodes;j++){
            if (is_link(g1) > p_degree){//random number between 0 to 100, less than degree of parallelism, make a link.
                adj_matrix[i][j] = random_com(g1);
            }

        }

    //if there are extra edges between nodes, remove them. For example, if i has an edge to j and k, and j has an edge to k, remove the edge from i to k.
    bool update = true;

    vector<unordered_set<int> > dep_list;
    dep_list.resize(num_nodes);
    while (update){
        update = false;

        for (int i=0;i<num_nodes;i++)
            for (int j=i+1;j<num_nodes;j++){
                if (adj_matrix[i][j] != -1){
                    if (dep_list[j].count(i) == 0){
                        update = true;
                        dep_list[j].insert(i);}

                }
                for (auto it = dep_list[i].begin(); it != dep_list[i].end(); it++)
                {
                    if (dep_list[j].count(*it) == 0){
                        dep_list[j].insert(*it);
                        update = true;
                    }
                }
            }
    }

    for (int i=0;i<num_nodes;i++)
        for (int j=i+1;j<num_nodes;j++)
            for (int k=j+1;k<num_nodes;k++){
                //if k depends on j and j depends on i, remove the k to i dependency
                if(dep_list[j].count(i) != 0 && dep_list[k].count(i) != 0 && dep_list[k].count(j) !=0){
                    adj_matrix[i][k] = -1;
                }
            }
   //the last node should depend on all nodes. if it doesn't create a direct one
    for (int i=0;i<num_nodes-1;i++)
        if (dep_list[num_nodes-1].count(i) == 0){
        adj_matrix[i][num_nodes-1] = random_com(g1);
        }


}

