#ifndef vis_h
#define vis_h
#include "computeResource.h"
#include "workflowgraphs.h"
#include <iostream>
#include <limits>
#include <fstream>
#include <sstream>
#include <cassert>
#include <unordered_map>
#include <stdlib.h>
#include "visualizer.h"
#include "globals.h"

using namespace std;


//this function dumps a .dot file for graphviz and then performs a system call to produce a pdf file using graphviz
void visualize_compute(){
    //output stream
    ofstream fout;
    //only do it if graphviz is used
    if (usegraphviz){
        //open the file for writing
        fout.open("resources.dot");
        //directional graph
        fout <<"digraph{"<<endl;
        //draw it left to right (LR)
        fout<<"rankdir=\"LR\""<<endl;
        //for every resource, create a node with its name as label
        for (int ii=0;ii<resources.size();ii++){
            fout<<"R"<<ii<<" [label="<<resources[ii].getName()<<"]"<<endl;
        }
        //for all resourcs to the others, create edges sing the speed
        for (int ii = 0; ii < resources.size(); ii++){
            for (int jj = 0; jj < resources.size(); jj++){
                if (ii == jj) continue;
                if (links[ii][jj].getSpeed() == links[jj][ii].getSpeed() && links[ii][jj].getLatency() == links[jj][ii].getLatency()){
                    if (ii < jj)
                        if (links[ii][jj].getSpeed()<1000)
                            fout<<"R"<<ii<<" -> "<<"R"<<jj<<" [dir=both, label=\""<<links[ii][jj].getSpeed()<<" Mbps, "<<links[ii][jj].getLatency()<<"ms\"]"<<endl;
                        else
                            fout<<"R"<<ii<<" -> "<<"R"<<jj<<" [dir=both, label=\""<<links[ii][jj].getSpeed()/1000<<" Gbps, "<<links[ii][jj].getLatency()<<"ms\"]"<<endl;
                    }
                else{
                    if (links[ii][jj].getSpeed()<1000)
                        fout<<"R"<<ii<<" -> "<<"R"<<jj<<" [label=\""<<links[ii][jj].getSpeed()<<" Mbps, "<<links[ii][jj].getLatency()<<"ms\"]"<<endl;
                    else
                        fout<<"R"<<ii<<" -> "<<"R"<<jj<<" [label=\""<<links[ii][jj].getSpeed()/1000<<" Gbps, "<<links[ii][jj].getLatency()<<"ms\"]"<<endl;
                    }
            }
        }
        fout<<"}"<<endl;
        //close the file
        fout.close();
    }
    //system call to graphviz
    system("dot -Tpdf resources.dot -o resources.pdf");

}
//this function uses graphviz to visualize the workflow graphs
void visualize_wfgs(){
    
    //output stream file
    ofstream fout;
    //dot file
    fout.open("work.dot");
    //directional graph
    fout<<"digraph W0{"<<endl;
    //left to right
    fout<<"rankdir=\"LR\""<<endl;

    //subgraph for each of the workflow graphs 
    for (int gr = 0; gr<wfgs.size();gr++){
        fout<<"subgraph cluster_"<<gr<<"{"<<endl;
        //label is the name of the graph
        fout<<"label=\""<<wfgs[gr].getName()<<"\""<<endl;
        for (int ii=0; ii<wfgs[gr].getSize();ii++){
            //for each task, write a node
            if (wfgs[gr].getpt(ii) < 1000)
                fout<<"    T"<<gr<<"x"<<ii<<" [label=<"<<wfgs[gr].getNameat(ii)<<"<BR />"<<wfgs[gr].getpt(ii)<< " ms>];"<<endl;    
            else
                fout<<"    T"<<gr<<"x"<<ii<<" [label=<"<<wfgs[gr].getNameat(ii)<<"<BR />"<<wfgs[gr].getpt(ii)/1000.0<< " s>];"<<endl;    

        }
        for (int ii=0; ii<wfgs[gr].getSize();ii++){
            for (int jj=0;jj<wfgs[gr].getSize();jj++){
                if (wfgs[gr].get_matrix(ii,jj)<0)
                    continue;
                //if there is a connection between two tasks, create an edge
                if (wfgs[gr].get_matrix(ii,jj) < 1)
                    fout<<"    T"<<gr<<"x"<<ii<<" -> T"<<gr<<"x"<<jj<<" [label=<"<<wfgs[gr].get_matrix(ii,jj)*1000<<" B>]"<<endl;
                else if (wfgs[gr].get_matrix(ii,jj) < 1000)
                    fout<<"    T"<<gr<<"x"<<ii<<" -> T"<<gr<<"x"<<jj<<" [label=<"<<wfgs[gr].get_matrix(ii,jj)<<" KB>]"<<endl;
                else
                    fout<<"    T"<<gr<<"x"<<ii<<" -> T"<<gr<<"x"<<jj<<" [label=<"<<wfgs[gr].get_matrix(ii,jj)/1000.0<<" MB>]"<<endl;

            }
        }

        fout<<"}"<<endl;
    }
    fout<<"}"<<endl;
    fout.close();

    //invoke graphviz to create a pdf file
    system("dot -Tpdf work.dot -o work.pdf");
}

//this function uses graphviz to visualize the graph post transformation.
//the main difference with the previous function is that it includes communication tasks.
void visualize_transformed_graphs(){
    //open the file
    ofstream fout("work2.dot");
    //visualize the transformed graph
    vector<string> lines_for_transformed_graph;
    //its a direcitonal graph
    fout<<"digraph W0{"<<endl;
    //from left to right (LR)
    fout<<"rankdir=\"LR\""<<endl;
    //for all graphs, call the visualize tranformed matrix function, which returns strings we need to put into the file for that graph
    for (int i=0;i<wfgs.size();i++){
        lines_for_transformed_graph = wfgs[i].visualize_transformed_matrix();
        for (int j=0;j<lines_for_transformed_graph.size();j++)
            fout<<lines_for_transformed_graph[j]<<endl;
    }
    //end
    fout<<"}"<<endl;
    //close file
    fout.close();
    //invoke graphviz
    system("dot -Tpdf work2.dot -o work2.pdf");

}

//visualize the output using gnuplot.
//it shows at which time, the tasks were running on what resources
//it requires gnuplot on the system
//it starts by writing the script, moves to writing the data file and finally invokes gnuplot
void visualize_output_compute(){

    //the script first, data file next
    ofstream fout("vis_o.plt");
    //reset the terminal
    fout<<"reset"<<endl;
    //set type of output
    fout<<"set terminal postscript dashed color size 6,4"<<endl;
    //set name of outputfile
    fout<<"set output \"vis_o.eps\""<<endl;
    //set border
    fout<<"set border linewidth 1"<<endl;
    //now for all tasks in all graphs, we need to create a linestyle. If color is specified, tie it to that color
    int counter = 0;
    for (int i=0;i<wfgs.size();i++){
        for (int j=0;j<wfgs[i].getNumNodes();j++){
    fout<<"set style line "<<++counter<<" lc rgb \'#"<<wfgs[i].getColorAt(j)<<"\' lt 1 lw 20 pt 0 pi 0 ps 1"<<endl;
        }
    }
    //too many keys, so unset it
    fout<<"unset key"<<endl;
    //we need a y label every 1 index in the Y axis, set font
    fout<<"set ytics 1 font \"Helvetica,9\""<<endl;
    //set xtics font and label
    fout<<"set xtics font \"Heltiva,9\" offset 0,0.5"<<endl;
    fout<<"set xlabel \"Time (ms)\" offset 0,1"<<endl;
    fout<<"set xlabel font \"Helvetica,9\""<<endl;
    //y range depends on resources
    fout<<"set yrange [-0.5:"<<resources.size()<<"]"<<endl;
    //ytics, name of all resources
    fout<<"set ytics(";
    for (int i=0;i<resources.size();i++)
        fout<<"\""<<resources[i].getName()<<"\" "<<i+0.5<<", ";
    fout <<");"<<endl;
    counter = 1;
    //for all computation nodes in all graphs, draw using the style we defined before
    for (int i=0;i<wfgs.size();i++){
        for (int j=0;j<wfgs[i].getNumNodes();j++){
            //first and last one look different
            if (i==0 && j==0){//first
                fout<<"plot \'vis_o.dat\' index 0 with lines linestyle 1, \\"<<endl;
            }
            else if (i== wfgs.size()-1 && j==wfgs[i].getNumNodes()-1){//last
                fout<<"     ''                   index "<<counter++<<" with lines linestyle "<< counter<<endl;
            }
            else{
                fout<<"     ''                   index "<<counter++<<" with lines linestyle "<<counter<<",\\"<<endl;
            }
        }
    }
    //close file
    fout.close();
    //open the data file
    fout.open("vis_o.dat");
    //for all nodes in workflow graphs
    for (int i=0;i<wfgs.size();i++){
        for (int j=0;j<wfgs[i].getNumNodes();j++){
            //find x and y indices based on runtime and assigned resource, and write it in the data file
            double y_index = wfgs[i].findResource(j) + 0.5;
            double x_start = wfgs[i].findStart(j);
            double x_end = wfgs[i].findEnd(j);
            fout<<x_start<<" "<<y_index<<endl;
            fout<<x_end<<" "<<y_index<<endl;
            fout<<endl;
            fout<<endl;
        }
    }
    //close file
    fout.close();
    //invoke gnuplot
    system("gnuplot vis_o.plt");
}

//visualize links and their usage using gnuplot
//first write the script, then the data file, and lastly do a system call and invoke gnuplot
//requires gnuplot to be installed on the system
void visualize_output_links(){
    //the script first, data file next
    //open the script file
    ofstream fout("link_o.plt");
    //reset the terminal
    fout<<"reset"<<endl;
    //set terminal type
    fout<<"set terminal postscript dashed color size 6,4"<<endl;
    //set output name
    fout<<"set output \"link_o.eps\""<<endl;
    //set border line width
    fout<<"set border linewidth 1"<<endl;
    int counter = 0;

    //for all workflow graphs, get all their tasks and create a line style for their assigned color
    for (int i=0;i<wfgs.size();i++){
        for (int j=wfgs[i].getNumNodes();j<wfgs[i].getNumTasks();j++){
            
            fout<<"set style line "<<++counter<<" lc rgb \'#"<<wfgs[i].getColor()<<"\' lt 1 lw 10 pt 0 pi 0 ps 1"<<endl;
        }
    }
    //unset key as it gets too messy with many tasks
    fout<<"unset key"<<endl;
    //we want to see all y axis tics, set font
    fout<<"set ytics 1 font \"Helvetica,9\""<<endl;
    //set xtics font and label and label
    fout<<"set xtics font \"Heltiva,9\" offset 0,0.5"<<endl;
    fout<<"set xlabel \"Time (ms)\" offset 0,1"<<endl;
    fout<<"set xlabel font \"Helvetica,9\""<<endl;
    //y range depends on number of resources
    fout<<"set yrange [-0.5:"<<resources.size()*(resources.size()-1)<<"]"<<endl;
    //names for all ytics
    fout<<"set ytics(";
    int reduce_off_set = 0;
    for (int i=0;i<resources.size();i++)
        for (int j=0;j<resources.size();j++)
        {   
            if (i==j){//same resource to same resource doesn't have a link
                reduce_off_set++;
                continue;
                }
                //index of the link
            fout<<"\""<<resources[i].getName()<<"->"<<resources[j].getName()<<"\" "<<i*(resources.size())+j -reduce_off_set  +0.5<<", ";
        }
    fout <<");"<<endl;
    counter = 1;
    //plot the tasks, one by one
    bool first = true;
    for (int i=0;i<wfgs.size();i++){
        for (int j=wfgs[i].getNumNodes();j<wfgs[i].getNumTasks();j++){
            //first and last task look different
            if (first && j==wfgs[i].getNumNodes()){//first task
                fout<<"plot \'link_o.dat\' index 0 with lines linestyle 1, \\"<<endl;
                first = false;
            }
            else if (i== wfgs.size()-1 && j==wfgs[i].getNumTasks()-1){//last task
                fout<<"     ''                   index "<<counter++<<" with lines linestyle "<< counter<<endl;
            }
            else{
                fout<<"     ''                   index "<<counter++<<" with lines linestyle "<<counter<<",\\"<<endl;
            }
        }
    }
    //close the script file
    fout.close();
    //open the data file
    fout.open("link_o.dat");
    //for all communication tasks, find their starting and end times and their resource.
    //create proper x and y coordinates for the lines to draw, print to file
    for (int i=0;i<wfgs.size();i++){
        for (int j=wfgs[i].getNumNodes();j<wfgs[i].getNumTasks();j++){
            double y_index = wfgs[i].findResource(j) + 0.5;
            double x_start = wfgs[i].findStart(j);
            double x_end = wfgs[i].findEnd(j);
            fout<<x_start<<" "<<y_index<<endl;
            fout<<x_end<<" "<<y_index<<endl;
            fout<<endl;
            fout<<endl;
        }
    }

    //close data file
    fout.close();
    //invoke gnuplot
    system("gnuplot link_o.plt");

}

#endif
