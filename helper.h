
#include "computeResource.h"
#include "workflowgraphs.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cassert>
#include <unordered_map>
#include <bits/stdc++.h>
#include "globals.h"

using namespace std;


//test
void update_bit_rl(){
    if (rand()%100 == 0){
        selected_bit = rand()%(max_bf_size+1);
    }
    else{
        selected_bit = 0;
        double highest_success = 0;
        for (int i=0;i<success_bf.size();i++){
            if (success_bf[i]/total_bf[i] > highest_success){
                highest_success = success_bf[i]/total_bf[i];
                selected_bit = i;
            }

        }
    }

}

void print_cost_to_file(){
    ofstream outfile;
    outfile.open("time.dat", std::ios_base::app);
    outfile<<print_helper<<" "<<best_time<<endl;
    outfile.close();
    outfile.open("power.dat", std::ios_base::app);
    outfile<<print_helper<<" "<<best_pow<<endl;
    outfile.close();
    outfile.open("cost.dat", std::ios_base::app);
    outfile<<print_helper<<" "<<best_cost<<endl;
    outfile.close();
    outfile.open("total.dat",std::ios_base::app);
    outfile<<print_helper++<<" "<<best_time * time_weight + best_cost * cost_weight + best_pow * energy_weight<<endl;
    outfile.close();
}
//this is the cost function
//it calculates a weighted sum of monetary cost, runtime, and energy
//weights are given by the user

double calculate_total_cost(){
    //comment to avoid this

    return time_spent * time_weight + cost * cost_weight + energy * energy_weight;
}



//parse all workflow graphs to update monetary cost and time ran so far
double parse_for_cost_data(){
    double total_local_time = 0;
    double total_cost = 0;
    double time_max = 0;
    //for all graphs
    for (int gr =0; gr<wfgs.size();gr++){
        //update costs
        wfgs[gr].update_costs(total_local_time,total_cost);
        if (wfgs[gr].getTime() > time_max)
            time_max = wfgs[gr].getTime();

    }
    //calculate energy
    energy = resources[0].getPow() * total_local_time + resources[0].getIdlePow() *(time_max - total_local_time);
    //calculate time
    time_spent = time_max;
    //update the cost variable 
    cost = total_cost;
    //invoke function that calculates total cost based on weighted averages
    return calculate_total_cost();

}

//bruteforce assignments
bool bf_gen_next_assignment(const vector<int>& orig_assignments, vector<int> & assignments){
    //for bruteforce we "ripple" an increment in the assignments
    for (int i=0;i<assignments.size();i++){
        if (orig_assignments[i] != -1)
            continue;
        if (assignments[i] == resources.size() - 1){
            assignments[i] = 0;
            continue;
        }
        else{   
            assignments[i]++;
            return true;
        }
    }
    //everything has been tested
    return false;
}

//randomly generate the next assignment
bool rand_gen_next_assignment(const vector<int> & orig_assignments,vector<int> & assignments, int& current_count, int max_count){
    if (current_count < max_count){
        for (int i=0;i<assignments.size();i++){
            if (orig_assignments[i] != -1)
                continue;
            assignments[i] = rand()% resources.size();
        }
        current_count++;
        return true;
    }
    return false;
}

//this set of assignments is used when using approaches that need more than one solution, such as the hybrid approach below
vector< vector<int> > set_of_assignments;

//this is the hybrid bruteforce and random solution
//for the first 1/10 of iterations, it randomly generates solutions and we keep the best one
//for the remainder, it will randomly select 1,2, or 3 bits, and we bruteforce only those bits to all resources
bool int_gen_next_assignment(const vector<int> & orig_assignments,vector<int>&assignments, int& current_count, int max_count){
    if (set_of_assignments.size() != 0){//if we have pre-generated assignments, just use those
        assignments = set_of_assignments[set_of_assignments.size()-1];
        set_of_assignments.pop_back();
        current_count++;
        return true;
    }
    else if (current_count<max_count){//no pre-generated assignments, make new ones
        //choose one,two, three, or four randomly
        int bits = rand()% max_bf_size + 1;
        vector<int> indices;
        while (indices.size()<bits){
            int temp = rand()%assignments.size();
            if (orig_assignments[temp] == -1 && find(indices.begin(),indices.end(),temp) == indices.end()){
                indices.push_back(temp);
                assignments[temp] = 0;
            }
        }
        //we have selected the indices to bruteforce. Now find all possible combinations for those
        set_of_assignments.push_back(assignments);

        for (int i=0;i<assignments.size();){//bruteforce only selected indices
            if (orig_assignments[i] != -1 || find(indices.begin(),indices.end(),i) == indices.end())
                {   i++;
                    continue;
                }
            if (assignments[i] == resources.size() - 1){
                assignments[i] = 0;
                bool more_work = false;
                for (int z=i+1;z<assignments.size();z++){
                    if (assignments[z] == -1 || find(indices.begin(),indices.end(),z) == indices.end())
                        continue;
                    if (assignments[z] <resources.size() -1){
                        assignments[z]++;
                        more_work=true;
                        break;
                    }
                    else{
                        assignments[z] = 0;
                    }
                }

                if(!more_work)
                    break;
                set_of_assignments.push_back(assignments);
                continue;
            }
            else{   
                assignments[i]++;
                set_of_assignments.push_back(assignments);
            }
        }

        
        //use the last assignment in the list
        assignments = set_of_assignments[set_of_assignments.size()-1];
        set_of_assignments.pop_back();
        current_count++;
        return true;
       
    }
    return false;
}

//bruteforce helper
//do bruteforce on select number of bits.
bool rl_gen_next_assignment(const vector<int> & orig_assignments,vector<int>&assignments, int& current_count, int max_count){

    if (set_of_assignments.size() != 0 && current_count<max_count){//if we have pre-generated assignments, just use those
        assignments = set_of_assignments[set_of_assignments.size()-1];
        set_of_assignments.pop_back();
        current_count++;
        return true;
    }
    else if (current_count<max_count){//no pre-generated assignments, make new ones
        update_bit_rl();
        int bits = selected_bit;
        if (bits == 0){
            return rand_gen_next_assignment(orig_assignments,assignments,current_count,max_count);
        }
        vector<int> indices;
        while (indices.size()<bits){
            int temp = rand()%assignments.size();
            if (orig_assignments[temp] == -1 && find(indices.begin(),indices.end(),temp) == indices.end()){
                indices.push_back(temp);
                assignments[temp] = 0;
            }
        }
        //we have selected the indices to bruteforce. Now find all possible combinations for those
        set_of_assignments.push_back(assignments);

        for (int i=0;i<assignments.size();){//bruteforce only selected indices
            if (orig_assignments[i] != -1 || find(indices.begin(),indices.end(),i) == indices.end())
                {   i++;
                    continue;
                }
            if (assignments[i] == resources.size() - 1){
                assignments[i] = 0;
                bool more_work = false;
                for (int z=i+1;z<assignments.size();z++){
                    if (assignments[z] == -1 || find(indices.begin(),indices.end(),z) == indices.end())
                        continue;
                    if (assignments[z] <resources.size() -1){
                        assignments[z]++;
                        more_work=true;
                        break;
                    }
                    else{
                        assignments[z] = 0;
                    }
                }

                if(!more_work)
                    break;
                set_of_assignments.push_back(assignments);
                continue;
            }
            else{   
                assignments[i]++;
                set_of_assignments.push_back(assignments);
            }
        }
        
        assignments = set_of_assignments[set_of_assignments.size()-1];
        set_of_assignments.pop_back();
        current_count++;
        return true;
       
    }
    return false;
}



//generate the next assignment genetic algorithm mutations
bool ga_dvfs_mutation_gen_next_assignment(const vector<int> & orig_assignments,vector<int>&assignments, int& current_count, int max_count){
    if (current_count < max_count){
        bool success = false;
        while (!success){
            int index = rand()% assignments.size();
            if (index < assignments.size()>>1)
            {
                if (orig_assignments[index] == -1){
                    assignments[index] = rand()%resources.size();
                    success = true;
                }
            }
            else{
                int resource_used = assignments[index- (assignments.size()>>1)];
                if (resources[resource_used].getType() == 1){
                    assignments[index] =rand()%resources[resource_used].getspratios().size(); 
                }
            }
        }
        current_count++;
        return true;
    }
    return false;


}

//generate the next assignment genetic algorithm mutations
bool ga_mutation_gen_next_assignment(const vector<int> & orig_assignments,vector<int>&assignments, int& current_count, int max_count){
    if (current_count < max_count){
        bool success = false;
        while (!success){
            int index = rand()% assignments.size();
            if (orig_assignments[index] == -1){
               assignments[index] = rand()%resources.size();
               success = true;
            }
        }
        current_count++;
        return true;
    }
    return false;
}



// this function assumes that assignments are valid and does all the repetitive work of running the tasks in the graphs
void runWithAssignments(){

    //find number of resources
    int num_resources = resources.size();
    //is there more work to be done?
    bool more_work = true;

    //reset all the graphs and then transform them so they are ready to be run again
    for (int i=0;i<wfgs.size();i++){
        wfgs[i].reset();
        wfgs[i].transform_graph();
    }

    //while there's more work to do
    while (more_work){
        
        //if nothing done this iteration, we want to leave the loop
        more_work = false;

        //first, ask all graphs to update their runnable tasks
        for (int i=0;i<wfgs.size();i++){
            if (wfgs[i].graphdone()){
                continue;
            }
            wfgs[i].findRunnableTasks();
        }

        //now ask them to start running tasks based on available resources;
        for (int i=0;i<resources.size();i++){
            if (resources[i].checkBusy())
                continue;
            for (int j=0;j<wfgs.size();j++){
                if (resources[i].checkBusy())
                    continue;
                if (wfgs[j].graphdone()== false){
                    if(wfgs[j].startRunningTasks(0,i)){
                        resources[i].getBusy();
                        //j=wfgs.size();//end this loop
                    }
                }
            }
        }

        //now do the same for links:
        for (int i=0;i<links.size();i++){
            for (int k =0;k<links.size();k++){
                if (links[i][k].checkBusy()){
                    continue;
                }
                for (int j=0;j<wfgs.size();j++){
                    if (wfgs[j].graphdone()== false){

                        if(wfgs[j].startRunningTasks(1,i*num_resources+k+num_resources)){
                            links[i][k].getBusy();
                            j=wfgs.size();//end this loop
                        }
                    }
                }
            }
        }


        //helpful for debug
        
        /*
        for (int i=0;i<wfgs.size();i++){
        wfgs[i].printevents();
        wfgs[i].printstats();
        }
        */
         
        //find the smallest remaining task, and have everyone run that much
        //initialize it to the largest double
        double smallest = numeric_limits<double>::max();
        for (int i=0;i<wfgs.size();i++){
            //if the graph has more work to do
            if (wfgs[i].graphdone() == false){
                //find the smallest running task of that graph
                double temp = wfgs[i].findSmallestRunningTask();
                if (temp < smallest)
                    smallest = temp;
            }
        }
        //the smallest task should be smaller than the largest double or we failed
        if (smallest == numeric_limits<double>::max())
        {
            cout<<"This should never happen"<<endl;
            //this kills the program
            assert(0);
        }
        //make all the graphs that are not done run all their tasks by the smallest task amount
        for (int i=0;i<wfgs.size();i++){
            if (wfgs[i].graphdone() == false){
                wfgs[i].runTasks(smallest);
            }
        }
        
        //check all the tasks to see if they are finished, if they are, free the resource
        for (int i=0;i<wfgs.size();i++){
            if (wfgs[i].graphdone() == false){
                vector<int> dones = wfgs[i].checkFinishedTasks();
                if (dones.size() == 0)
                    continue;
                for (int j=0;j<dones.size();j++){
                    if (dones[j]< resources.size())//computation task
                        resources[dones[j]].getFree();
                        
                    else{
                        //communication task. We need to extract link indices from this number
                        int decode = dones[j];
                        decode -= resources.size();
                        int first = decode/resources.size();
                        int second = decode%resources.size();
                        links[first][second].getFree();
                    }


                }
            }
        }


        //if all graphs are done, we are done
        for (int i=0;i<wfgs.size();i++){
            if (wfgs[i].graphdone())
                continue;
            else{
                more_work = true;
            }
        }
    }
}
