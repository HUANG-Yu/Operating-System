#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

struct operation {
    int no;
    int time; //arrive time
    int start_t; //start time
    int end_t;
    int track;
};

struct summary {
    int total_time;
    int tot_movement;
    double avg_turnaround;
    double avg_waittime;
    int max_waittime;
};