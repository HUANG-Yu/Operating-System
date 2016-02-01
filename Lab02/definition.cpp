#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

using namespace std;

struct Summary{
    int finish;
    double cpu_util;
    double io_util;
    double avg_turnaround;
    double avg_waittime;
    double throughput;
};

struct Process{
    int id; // id of the process
    int arrive_t; //arrival time
    int total_cpu_t; //total cpu burst
    int cpu_burst; //fixed cpu burst
    int io_burst; //fixed io burst
    int priority; //arrival time priority
};

struct Event{
    Process p;
    int finish_t; //finishing time
    int turnaround_t; //turnaround time = finishing time - arrival time
    int io_t; //time in blocked state - increment everytime from after io burst
    int ready_t; //cpu waiting time
    
    int rem; //remaining cpu time
    int randval; //the current generate cb or ib
    int time_stamp; //NOW + cb/ib
    string state;
    int rem_cpu_burst; //effective for RR and PR which have quantums
    
    int dynamic_p;
};