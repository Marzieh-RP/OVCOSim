#ifndef parser_h
#define parser_h
#include "xmlparser/pugixml.hpp"
#include "parser.h"
#include "globals.h"
#include "computeResource.h"
#include "workflowgraphs.h"
#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <cassert>
#include <string>
#include <cstring>
#include <unordered_map>

using namespace std;


//this file writes into variables defined in globals.h and trying to separate its implementation from definitions would produce linking errors.
//read the configuration file and parse general program settings
void parse_settings(string filename){
    //create an xml document
    pugi::xml_document doc;
    //load the file
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    //read the configs 
    pugi::xml_node configs = doc.child("config");

    //default values for optional parameters
    time_weight = 0;
    energy_weight = 0;
    cost_weight = 0;

    //parse general settings
    for (pugi::xml_attribute attr = configs.child("settings").first_attribute(); attr; attr = attr.next_attribute())
    {
        //every attribute has a value, parse those two here
        string this_name = attr.name();
        string this_value = attr.value();
        //if the attribute is the algorithm, see what it is
        if (this_name == "algorithm"){
            if (this_value=="randomized"){
                algorithm = randomized;
            }
            else if (this_value=="bruteforce"){
                algorithm = bruteforce;
            }
            else if (this_value=="rand_pb"){
                algorithm = rand_hybrid;
            }
            else if (this_value=="rl_pb"){
                algorithm = rl_hybrid;
            }
            else if (this_value=="mutation"){
                algorithm = ga;
            }
        }
        //if it's the number of iterations, read it and convert to integer
        else if (this_name == "iterations"){
            algorithm_iterations = stoi(this_value);
        }
        //does user want to use gnuplot?
        else if (this_name == "usegnuplot"){
            if (this_value == "on")
                usegnuplot = true;
        }
        //does user want to use graphviz
        else if (this_name == "usegraphviz"){
            if (this_value == "on")
                usegraphviz = true;
        }
        //read the weight for runtime and convert it to double
        else if (this_name == "speed_weight"){
            time_weight = stod(this_value);
        }
        //read the energy weight and conver to double
        else if (this_name == "energy_weight"){
            energy_weight = stod(this_value);
        }
        //read the cost weight and conver to double
        else if (this_name == "cost_weight"){
            cost_weight = stod(this_value);
        }

        else{
        //we need this warning to identify typos, etc
            cout<<"UNKNOWN ATTRIBUTE "<< attr.name()<<"WITH VALUE "<<attr.value()<<endl;
        }

    }
}


//this function parses the computation resources and links
void parse_compute(string filename){

    //create a document
    pugi::xml_document doc;
    //load the file into it
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    //read the confi settings
    pugi::xml_node configs = doc.child("config");
    //read resources
    pugi::xml_node xmlresources = configs.child("resources");
    //we need this hash table to map resource names to an index, so we can create links later
    unordered_map<string,int> temporary_name_to_index_for_links;
    //index for resources
    int temp_index = 0;
    //parse all resources
    for (pugi::xml_node xmlresource = xmlresources.first_child(); xmlresource; xmlresource = xmlresource.next_sibling())
    {
        //temp resource 
        computeResource t_comp;
        //parse all attributes of this resource
        for (pugi::xml_attribute attr = xmlresource.first_attribute(); attr; attr = attr.next_attribute())
        {
            string t_name = attr.name();
            string t_val = attr.value();
            //parse the name
            if (t_name == "name"){
                t_comp.setName(t_val);
                temporary_name_to_index_for_links[t_val] = temp_index;
                temp_index++;
            }
            //parse its power
            else if (t_name == "idlepowerratio")
            {
                t_comp.setPow(1.0);
                //obtaing the ratio of idle power to used power is all we need for the cost optimization
                t_comp.setIdlePow(stod(t_val));
            }
            //parse its speed
            else if (t_name == "speed")
                t_comp.setSpeed(stod(t_val));
            //parse its cost
            else if (t_name == "cost")
                t_comp.setCost(stod(t_val));
            else
                cout<<"UKNOWN "<<t_val<<"WITH VALUE"<<t_val<<endl;
        }
        //push the temp resource in
        resources.push_back(t_comp);
    }
    //set the 2d vector for links
    links.resize(resources.size());
    for (int i=0;i<links.size();i++)
        links[i].resize(resources.size());

    //parse the connections between computation networks
    for (pugi::xml_node xmlresource = xmlresources.first_child(); xmlresource; xmlresource = xmlresource.next_sibling())
    {
        //find the name
        string first_name = xmlresource.attribute("name").value();
        //parse all connections
        for (pugi::xml_node connection = xmlresource.first_child(); connection; connection = connection.next_sibling())
        {
            string d_name = "";
            double d_speed = 0;
            double d_latency = 0;
            //the hash table is used to find the index using the name
            int first_index = temporary_name_to_index_for_links[first_name];
            //pares all attributes
            for (pugi::xml_attribute attr = connection.first_attribute(); attr; attr = attr.next_attribute())
            {
                string t_name = attr.name();
                string t_val = attr.value();
                //link attributes, to what resource, at what speed and at what latency
                if (t_name == "to"){
                    d_name = t_val;
                }
                else if (t_name == "speed"){
                    d_speed = stod(t_val);
                }
                else if (t_name == "latency"){
                    d_latency = stod(t_val);
                }
                else
                    cout<<"UKNOWN"<<t_name<<"WITH VALUE"<<t_val<<endl;

            }
            //find the index of the other resource to make the link
            int second_index = temporary_name_to_index_for_links[d_name];
            //we can make the link now that we have all the info
            links[first_index][second_index].initialize(d_name, d_speed, d_latency);
        }

    }

}

//parse all workflow graphs
void parse_wfgs(string filename){
    //create an xml document
    pugi::xml_document doc;
    //load the file
    pugi::xml_parse_result result = doc.load_file(filename.c_str());
    //read all the configs
    pugi::xml_node configs = doc.child("config");
    //parse out the WFGs
    //one outter loop for each WFG
    pugi::xml_node xmlwfgs = configs.child("wfgs");
    for (pugi::xml_node xmlwfg = xmlwfgs.first_child(); xmlwfg; xmlwfg = xmlwfg.next_sibling())
    {

        string wfg_type = xmlwfg.attribute("type").value();
        if (wfg_type == "userdefined"){
            string wfg_name = xmlwfg.attribute("name").value();//extract WFG name
            string wfg_color = xmlwfg.attribute("color").value();//extract WFG color
            wfg temp(wfg_name);//temp wfg to push in after setting up
            //one inner loop for nodes of the WFG, with name to index mapping
            //hash table needed to map names to indices, used when creating connections between tasks
            unordered_map<string,int> name_to_index_map;
            int temp_index = 0;
            //temporary vectors of values, to be loaded into the workflowgraphs class
            vector<double> t_loads;
            vector<int> t_assignments;
            vector<string> t_colors;
            vector<string> t_names;
            //parse the tasks of the graph
            for (pugi::xml_node xmlnode = xmlwfg.first_child(); xmlnode; xmlnode =  xmlnode.next_sibling()){
                string node_name ="";
                string node_color = "000000";//black by default
                double node_compute = 0;
                int pre_assigned = -1;
                for (pugi::xml_attribute attr = xmlnode.first_attribute(); attr; attr = attr.next_attribute())
                {
                    //parse all attributes of the task
                    string t_name = attr.name();
                    string t_val = attr.value();
                    if (t_name == "name"){
                        node_name = t_val;
                    }
                    else if (t_name == "color"){
                        node_color = t_val;
                    }
                    else if (t_name == "compute"){
                        node_compute = stod(t_val);
                    }
                    else if (t_name == "assignment"){
                        string node_a = t_val;
                        //should have kept the hash table that I made in the other function for a quick lookup, but since there aren't that many resources I can just loop through
                        for (int i=0;i<resources.size();i++)
                            if (resources[i].getName() == node_a){
                                pre_assigned = i;
                            }
                    }
                    else
                        cout<<"UKNOWN"<<t_name<<"WITH VALUE"<<t_val<<endl;

                }
                //load up the parsed values into the vectors
                name_to_index_map[node_name] = temp_index;
                temp_index++;
                t_names.push_back(node_name);
                t_colors.push_back(node_color);
                t_loads.push_back(node_compute);
                t_assignments.push_back(pre_assigned);
            }
            //set up the wfg
            temp.setNumNodes(t_assignments.size());
            temp.setColor(wfg_color);
            temp.initializePT(t_loads);
            temp.initializeColors(t_colors);
            temp.initializeNames(t_names);
            temp.initializeAssignments(t_assignments);

            //one inner loop for outgoing edges from nodes of that WFG to each other
            temp.initialize_matrix();
            for (pugi::xml_node xmlnode = xmlwfg.first_child(); xmlnode; xmlnode =  xmlnode.next_sibling()){
                string node_name =xmlnode.attribute("name").value();
                for (pugi::xml_node xmlto = xmlnode.first_child(); xmlto; xmlto =  xmlto.next_sibling()){
                    string t_name = xmlto.attribute("name").value();
                    string t_data = xmlto.attribute("data").value();
                    double d_data = stod(t_data);
                    //this loads the d_data onto the edge going from node_name to t_name
                    temp.change_matrix(name_to_index_map[node_name], name_to_index_map[t_name],d_data);
                }

            }

            //the workflow graph "temp" is fully parsed and loaded, push it into workflow graphs and go to the next one, if any.
            wfgs.push_back(temp);
        }
        else{
            string g_name = xmlwfg.attribute("name").value();
            double g_nmean = stod(string(xmlwfg.attribute("nodes_mean").value()));
            double g_ndev =stod(string(xmlwfg.attribute("nodes_stddev").value()));
            double g_lmean =stod(string(xmlwfg.attribute("computation_mean").value())); 
            double g_ldev =stod(string(xmlwfg.attribute("computation_stddev").value())); 
            double g_cmean =stod(string(xmlwfg.attribute("communication_mean").value())); 
            double g_cdev =stod(string(xmlwfg.attribute("communication_stddev").value())); 
            int g_p = stoi(string(xmlwfg.attribute("parallelism").value()));
            string g_color =xmlwfg.attribute("color").value();
            int seed = stoi(string(xmlwfg.attribute("seed").value()));


            wfgs.push_back(wfg(g_name,g_nmean,g_ndev,seed,g_lmean,g_ldev,g_cmean,g_cdev,g_p,g_color));
        }
    }
    


}
#endif
