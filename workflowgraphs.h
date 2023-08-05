#ifndef wfg_h
#define wfg_h
#include <vector>
#include <limits>
#include <string>
#include <iostream>

using namespace std;


struct task{
    int type;//0 for computation, 1 for comunication
    int assigned_to;//the computation resource or link the task is assigned to
    double load;//workload before completion
};

struct event{
    int type;//0 for task start, 1 for end
    int task_num;//number of task
    double time;//when did this happen
};

class wfg{
    private:
        string name; //name of graph
        string color; //graph color. Used in visualizing links in the graph
        int num_nodes; //number of nodes in the graph before transformation
        vector<double> processing_time; // processing time needed for each graph. Used in the original graph before transformation only.
        vector<int> assignments; // assignments. This must be valid before transformation
        vector<vector<double> > adj_matrix; // communication between tasks. -1 implies no connection between the tasks, other values are the amount of data to be communicated

        /*variables below are used in the transformed graph*/
        vector<task> tasks; //tasks, can be both compute or communication
        vector<vector<bool> > waiting_on; //what is the task waiting on in form of a matrix
        vector<string> colors; // color of tasks for drawing
        vector<string> names; // name of tasks 
        vector<bool> tasks_ready_to_run; // tasks that are ready to run, in the transformed graph
        vector<bool> tasks_ran; // tasks that have finished running, in the transformed graph
        vector<bool> tasks_running; // tasks that are currently running.
        double total_time;//total time passed
        bool all_done;//nothing else left to do in this graph
        vector<event> events;//log of all events: start of tasks and end of tasks

        /* some information is needed about compute resources and links in this class, they are stored below*/
        int num_compute_resources; //number of computation resources
        vector<vector<double> >link_speeds; //speed of links
        vector<vector<double> >link_latencies; //latency of links
        vector<double> compute_speed;//speed of resources
        vector<double> compute_cost;//cost of resources
    public:
        wfg(); //default constructor
        wfg(string n, double nodes_avg, double nodes_dev, int seed, double load_avg, double load_dev, double communication_avg, double communication_std, int p_degree,string col); //used for random workflowgraph generation
        wfg(string n); //constructor with name
        void setNumComp(int n); //set number of computation resources
        void initialize_matrix(); //initialize the matrix using weights of tasks
        void transform_graph();//transform the graph so that communication tasks are created
        vector<string> visualize_transformed_matrix();//a vector of strings that can help visualize the graph using graphviz
        double get_matrix(int s, int d);//get the value of the matrix from s to d
        void setName(string n);//set name of graph
        void setNumNodes(int n);//set number of nodes
        int getNumNodes();//get number of nodes
        int getNumTasks();//get number of tasks

        void initializePT(const vector<double>& data);//initialize the processing time vector
        void initializeColors(const vector<string>& co);//initialize colors of tasks
        string getColorAt(int index);//get the color of task
        void setColor(string c);//set the graph color (used for links)
        string getColor();//get the graph color (used for links)
        void initializeNames(const vector<string>& n);//initialize the names of tasks

        void initializeAssignments(const vector<int>& assignment);//initialize assignments of tass

        void updateAssignments(const vector<int>& assignment, int start_index);//update a given assignment

        int getSize();// return size 
        double getpt(int i);// return processing time of index
        string getNameat(int i);//get name of index
        string getName();//get name of graph
        void setLinkSpeeds(const vector<vector<double> >& speeds);//set link speeds
        void setLinkLatencies(const vector<vector<double> >&latencies);//set link latencies
        void change_matrix(int s, int d, double load);//change a given location in the matrix
        void findRunnableTasks();//find all runnable tasks

        double findSmallestRunningTask();//find smallest running task
        bool startRunningTasks(int type, int resource_num);//start running task
        void runTasks(double time);//run all tasks for a given duration
        vector<int> checkFinishedTasks();//check for finished tasks
        bool graphdone();//is the graph done or is there more work to do
        vector<event> returnEvents();//return all events

        //for debug only
        void printevents();//prints internal info of the graph
        void printstats();//prints graph stats

        void resetTime();//resets time
        void setSpeeds(const vector<double>& speeds);//set speeds of resources
        void setCosts(const vector<double>& costs);//set costs of resources

        int findResource(int i);//find the resource at index
        double findStart(int i);//find start of task
        double findEnd(int i);//find end time of task
        double getTime();//get current time
        vector<int> getAssignments();//return all assignments
        void reset();//reset the graph so it's ready to be run again
        vector<string> visualize_stats();//help visualize the stats of the graph
        void update_costs(double& total_time_local, double& total_cost);//update the cost and local time parameters used in the cost function
};

#endif
