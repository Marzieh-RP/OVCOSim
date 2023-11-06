//* main module */
#include "computeResource.h"
#include "workflowgraphs.h"
#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include "xmlparser/pugixml.hpp"
#include "globals.h"
#include "parser.h"
#include "visualizer.h"
#include "helper.h"

using namespace std;








int main(int argc, char *argv[]){


    print_helper = 0;

    ofstream fout;
    string filename = "";
    // Read the settings
    if (argc != 2){
        cout <<"run the progrram as follows: .//binary_name configurationfile.xml";
        return 0;
    }
    else{
        filename = argv[1];
    }

    //test
    selected_bit = 0;
    total_bf.resize(max_bf_size+1);
    success_bf.resize(max_bf_size+1);
    for (int i=0;i<max_bf_size;i++){
        total_bf[i] = success_bf[i] = 1;
    }

    parse_settings(filename); //parse program settings
    parse_compute(filename); //parse compute network

    if (usegraphviz){
        visualize_compute(); //visualize compute network. This requires using GraphViz
    }

    parse_wfgs(filename); //parse the workflow graphs

    if (usegraphviz){
        visualize_wfgs(); //visualize workflow graphs. This requires using GraphViz
    }


    //resource speeds are needed for transforming the workflow graph, pass them here to all wfgs
    vector<double> resource_speeds;
    vector<double> resource_costs;
        unordered_set<int> all_mobiles;
    //gather information needed from resorces
    for (int i=0;i<resources.size();i++){
        resource_speeds.push_back(resources[i].getSpeed());
        resource_costs.push_back(resources[i].getCost());    
         if (resources[i].getType() == 1)
             all_mobiles.insert(i);
    }

    //create structures for link information to pass to work flow graphs
    vector<vector<double> >link_speeds;
    vector<vector<double> >link_latencies;
    link_speeds.resize(resources.size());
    link_latencies.resize(resources.size());
    //gather information needed from links
    for (int i=0;i<link_speeds.size();i++){
        link_speeds[i].resize(resources.size());
        link_latencies[i].resize(resources.size());
    }
    //pass link information to the data structures
    for (int i=0;i<resources.size();i++)
        for (int j=0;j<resources.size();j++){
            if (i==j) continue;
            link_speeds[i][j] = links[i][j].getSpeed();
            link_latencies[i][j] = links[i][j].getLatency();
        }

    //pass the information gathered above to the workflowgraphs
    for (int i=0;i<wfgs.size();i++){
        wfgs[i].setNumComp(resources.size());
        wfgs[i].setSpeeds(resource_speeds);
        wfgs[i].setCosts(resource_costs);
        wfgs[i].setLinkSpeeds(link_speeds);
        wfgs[i].setLinkLatencies(link_latencies);
        for (int j=0;j<resources.size();j++){
            if (wfgs[i].getMobileDevice() == resources[j].getName()){
                wfgs[i].setMobileDeviceList(all_mobiles,j);
                wfgs[i].setdvfs(resources[j].getspratios());
                }
        }
    }
    //combine all assignments into one:
    vector<int> all_assignments;
    for (int i=0;i<wfgs.size();i++){
        vector<int> temp = wfgs[i].getAssignments();
        all_assignments.insert(all_assignments.end(), temp.begin(), temp.end());
    }

    //in dvfs mode, we need to double the assignments.
    if (dvfs){
        vector<int> temp (all_assignments.size(),-1);
        all_assignments.insert(all_assignments.end(),temp.begin(),temp.end());
    }
    //all are assignments given by the user or do we need to find assignments?
    bool assignments_given = true;
    for (int i=0;i<all_assignments.size();i++)
        if (all_assignments[i] == -1)
            assignments_given = false;

    //original assignments, current assignments, best assignment so far and previous assignment
    vector<int> original = all_assignments;
    vector<int> current(all_assignments.size(),0);
    vector<int> best(all_assignments.size(),0);
    vector<int> prev(all_assignments.size(),0);

    //if all assignments are given, we only run once and we are done
    if (assignments_given)
        runWithAssignments();
    else{//find assignments!

        //initialize cost with a large value
        double best_costf = numeric_limits<double>::max();
        best_time = best_cost = best_pow;

        //initialize best, current, and prev assignments
        for (int i=0;i<original.size();i++){
            if (original[i] != -1)
            {
                current[i] = prev[i] = best[i] =  original[i];
            }
        }
        //pass assigments to each wfg
        int start_index = 0;
        for (int gr=0;gr<wfgs.size();gr++){
            wfgs[gr].updateAssignments(current,start_index);
            start_index += wfgs[gr].getAssignments().size();
        }
        //if dvfs, pass again
        if (dvfs)
        for (int gr = 0;gr<wfgs.size();gr++){
            wfgs[gr].updatePS(current,start_index);
            start_index += wfgs[gr].getAssignments().size();
        }

        //run the graphs
        runWithAssignments();
        //get the costs
        double this_cost = parse_for_cost_data();

        //is it a better cost?
        if (this_cost < best_costf){
            best_costf = this_cost;
            best_cost = cost;
            best_time = time_spent;
            best_pow = energy;
            best = current;
        }
        //update previous initially to best
        prev = best;
        print_cost_to_file();

        //bruteforce algorithm
        while (algorithm == bruteforce && bf_gen_next_assignment(original,current))
        {
            int temp_start = 0;
            for (int gr=0;gr<wfgs.size();gr++){
                wfgs[gr].updateAssignments(current,temp_start);
                temp_start = wfgs[gr].getAssignments().size();
            }

            runWithAssignments();
            double this_cost = parse_for_cost_data();

            if (this_cost < best_costf){
                best_costf = this_cost;
                best_cost = cost;
                best_time = time_spent;
                best_pow = energy;
                best = current;
            }
            print_cost_to_file();
        }

        //randomized algorithm
        int count_it = 0;
        while (algorithm == randomized && rand_gen_next_assignment(original,current,count_it, algorithm_iterations))
        {
            int temp_start = 0;
            for (int gr=0;gr<wfgs.size();gr++){
                wfgs[gr].updateAssignments(current,temp_start);
                temp_start = wfgs[gr].getAssignments().size();
            }

            runWithAssignments();
            double this_cost = parse_for_cost_data();

            if (this_cost < best_costf){
                best_costf = this_cost;
                best_cost = cost;
                best_time = time_spent;
                best_pow = energy;
                best = current;
            }
            else{
                current = best;
            }
            print_cost_to_file();
        }

        //hybrid of bruteforce and random
        while (algorithm == rand_hybrid && int_gen_next_assignment(original,current,count_it,algorithm_iterations)){

            int temp_start = 0;
            for (int gr=0;gr<wfgs.size();gr++){
                wfgs[gr].updateAssignments(current,temp_start);
                temp_start = wfgs[gr].getAssignments().size();
            }

            runWithAssignments();
            double this_cost = parse_for_cost_data();

            if (this_cost < best_costf){
                best_costf = this_cost;
                best_cost = cost;
                best_time = time_spent;
                best_pow = energy;
                best = current;
            }
            else{
                current = best;
            }
            print_cost_to_file();

        }
        while (algorithm == rl_hybrid && rl_gen_next_assignment(original,current,count_it,algorithm_iterations)){
            int temp_start = 0;
            for (int gr=0;gr<wfgs.size();gr++){
                wfgs[gr].updateAssignments(current,temp_start);
                temp_start = wfgs[gr].getAssignments().size();
            }

            runWithAssignments();
            double this_cost = parse_for_cost_data();

            if (this_cost < best_costf){
                best_costf = this_cost;
                best = current;
                success_bf[selected_bit] += 1;
                total_bf[selected_bit] += 1;
                best_cost = cost;
                best_time = time_spent;
                best_pow = energy;
            }
            else{
                current = best;
                total_bf[selected_bit] += 1;
            }
            print_cost_to_file();

        }
        //genetic algorithm mutations
        while (algorithm == ga){
        
        if (dvfs){
        if (ga_dvfs_mutation_gen_next_assignment(original,current,count_it,algorithm_iterations) == false)
        break;
        }
        else{
        if (ga_mutation_gen_next_assignment(original,current,count_it,algorithm_iterations) == false)
        break;
        }

            int temp_start = 0;
            for (int gr=0;gr<wfgs.size();gr++){
                wfgs[gr].updateAssignments(current,temp_start);
                temp_start = wfgs[gr].getAssignments().size();
            }

            //if dvfs, pass again
            start_index = start_index >> 1;
            if (dvfs)
            for (int gr = 0;gr<wfgs.size();gr++){
                wfgs[gr].updatePS(current,start_index);
                start_index += wfgs[gr].getAssignments().size();
            }

            runWithAssignments();
            double this_cost = parse_for_cost_data();

            if (this_cost < best_costf){
                best_costf = this_cost;
                best = current;
                best_cost = cost;
                best_time = time_spent;
                best_pow = energy;
            }
            else{
                current = best;
            }
            print_cost_to_file();

        }

        {
            //rerun best
            int temp_start = 0;
            for (int gr=0;gr<wfgs.size();gr++){
                wfgs[gr].updateAssignments(best,temp_start);
                temp_start = wfgs[gr].getAssignments().size();
                /*
                   for (int debug = 0;debug<wfgs[gr].getAssignments().size();debug++){
                   cout<<wfgs[gr].getAssignments()[debug];

                   }
                   cout<<endl;*/
            }

            runWithAssignments();
        }

    }

    // Print the Results in the terminal. not needed anymore since I create PDFs
    // But if the user doesn't have gnuplot, they still want to see it
    if (!usegnuplot){
        vector<string> events_gn;
        for (int i=0;i<wfgs.size();i++){
            cout<<"WFG "<<wfgs[i].getName()<<endl;
            wfgs[i].printevents();
        }
    }

    
    

    if (usegnuplot){
        visualize_output_compute();
        visualize_output_links();
    }

    for (int i=0;i<wfgs.size();i++){
        wfgs[i].reset();
        wfgs[i].transform_graph();}
    visualize_transformed_graphs();
    /*
    for (int i=0;i<success_bf.size();i++)
    {
        cout<<success_bf[i]<<" "<<total_bf[i]<<endl;
    }
    */

    return 0;
}



