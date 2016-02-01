#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <deque>
#include "definition.cpp"

using namespace std;

char * inputfile;
FILE * pFile;
ifstream rFile;
char read_char; //read process info from input file
string read_rstring; //read random number from rfile
int num_process; //record the num of process in a scheduler
int current_t = 0; // record the current time stamp for the scheduler
int no_it = 0; //accumulate the time where no process doing io
int quantum = 2;
deque<Event> global_queue;
deque<Event> event_queue;
deque<Event> ready_queue;
deque<Event> result_queue;
deque<Event> expired_queue; // prio
Summary process_info; //print the summary information of all processes

int myrandom(int burst){
    stringstream ss;
    int temp;
    getline(rFile, read_rstring);//get the priority from rfile
    ss << read_rstring;
    ss >> temp;
    ss.clear();
    return 1 + (temp % burst);
}

void skip_delimiter(){//skip token delimiters
    while (read_char == ' ' || read_char == '\n'||read_char == '\t') {
        read_char = fgetc(pFile);
    }
    return;
}

string read_int(){//read symbol relative address by concatenating each legal character
    string temp = "";
    while (read_char != ' ' && read_char != '\n' && read_char != '\t' && read_char != EOF) {
        temp = temp + read_char;
        read_char = fgetc(pFile);
    }
    return temp;
}

Event bc_ready(Event temp){
    current_t = temp.time_stamp;
    //cout<< current_t << " " << temp.p.id << " " << temp.randval << ": " << temp.state << " -> " << "READY" << endl;
    temp.state = "READY";
    return temp;
}

Event ready_running(Event temp){
    temp.randval = current_t - temp.time_stamp;
    temp.ready_t = temp.ready_t + temp.randval;
    temp.time_stamp = current_t;
    //cout<< current_t << " " << temp.p.id << " " << temp.randval << ": " << temp.state << " -> " << "RUNNING ";
    temp.randval = myrandom(temp.p.cpu_burst);
    if (temp.rem < temp.randval) {
        temp.randval = temp.rem;
    }
    //cout<< "cb=" << temp.randval << " rem=" << temp.rem << " prio=" << temp.p.priority << endl;
    temp.rem = temp.rem - temp.randval;
    temp.time_stamp = current_t + temp.randval;
    temp.state = "RUNNING";
    return temp;
}

Event run_quantum(Event temp){
    temp.randval = current_t - temp.time_stamp;
    temp.ready_t = temp.ready_t + temp.randval;
    temp.time_stamp = current_t;
    //cout<< current_t << " " << temp.p.id << " " << temp.randval << ": " << temp.state << " -> " << "RUNNING ";
    if (temp.rem_cpu_burst == 0) {
        temp.rem_cpu_burst = myrandom(temp.p.cpu_burst);
        if (temp.rem_cpu_burst <= quantum) {
            if (temp.rem <= temp.rem_cpu_burst) {
                temp.randval = temp.rem;
                //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
                temp.rem = 0;
            }
            else{
                temp.randval = temp.rem_cpu_burst;
                //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
                temp.rem = temp.rem - temp.randval;
            }
            temp.time_stamp = current_t + temp.randval;
            temp.rem_cpu_burst = 0;
        }
        else{ // randval > quantum
            if (temp.rem <= temp.rem_cpu_burst && temp.rem <= quantum) {
                temp.randval = temp.rem;
                //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
                temp.rem = 0;
            }
            else{
                temp.randval = quantum;
                //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
                temp.rem = temp.rem - quantum;
            }
            temp.time_stamp = current_t + temp.randval;
            temp.rem_cpu_burst = temp.rem_cpu_burst - quantum;
        }
    }
    else{
        if (temp.rem_cpu_burst - quantum >= 0) {
            if (temp.rem <= temp.rem_cpu_burst && temp.rem <= quantum) {
                temp.randval = temp.rem;
                //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
                temp.rem = 0;
            }
            else{
                temp.randval = quantum;
                //cout<< "cb=" << temp.randval << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
                temp.rem = temp.rem - quantum;
            }
            temp.time_stamp = current_t + temp.randval;
            temp.rem_cpu_burst = temp.rem_cpu_burst - quantum;
        }
        else{
            if (temp.rem <= temp.rem_cpu_burst) {
                temp.randval = temp.rem;
                //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
                temp.rem = 0;
            }
            else{
                temp.randval = temp.rem_cpu_burst;
                //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
                temp.rem = temp.rem - temp.randval;
            }
            temp.time_stamp = current_t + temp.randval;
            temp.rem_cpu_burst = 0;
        }
    }
    temp.state = "RUNNING";
    return temp;
}

Event running_ready(Event temp){
    current_t = temp.time_stamp;
    //cout<< current_t << " " << temp.p.id << " " << temp.randval << ": " << temp.state << " -> " << "READY ";
    //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.p.priority << endl;
    temp.state = "READY";
    return temp;
}

Event running_readyp(Event temp){
    current_t = temp.time_stamp;
    //cout<< current_t << " " << temp.p.id << " " << temp.randval << ": " << temp.state << " -> " << "READY ";
    //cout<< "cb=" << temp.rem_cpu_burst << " rem=" << temp.rem << " prio=" << temp.dynamic_p << endl;
    temp.state = "READY";
    return temp;
}

Event running_block(Event temp){
    current_t = temp.time_stamp;
    //cout<< current_t << " " << temp.p.id << " " << temp.randval << ": " << temp.state << " -> " << "BLOCKED ";
    temp.randval = myrandom(temp.p.io_burst);
    temp.io_t = temp.io_t + temp.randval;
    //cout<< "ib=" << temp.randval << " rem=" << temp.rem << endl;
    temp.state = "BLOCKED";
    temp.time_stamp = current_t + temp.randval;
    return temp;
}

Event done(Event temp){
    current_t = temp.time_stamp;
    //cout<< current_t << " " << temp.p.id << " " << temp.randval << ": Done" << endl;
    temp.state = "Done";
    temp.finish_t = current_t;
    return temp;
}

void insert_eq(Event temp){
    deque<Event>::iterator eq_it = event_queue.begin();
    while (eq_it != event_queue.end() && temp.time_stamp >= eq_it->time_stamp) {
        if (temp.time_stamp > eq_it->time_stamp) {
            eq_it++;
        }
        else{
            if (eq_it != event_queue.end() && temp.randval <= eq_it->randval) {
                eq_it++;
            }
        }
    }
    eq_it = event_queue.insert(eq_it, temp);
    return;
}

void insert_rq(Event temp){ //insert end process into result queue
    deque<Event>::iterator rq_it = result_queue.begin();
    while (rq_it != result_queue.end() && temp.p.id > rq_it->p.id) {
        rq_it++;
    }
    rq_it = result_queue.insert(rq_it, temp);
    return;
}

void insert_readyq(Event temp){
    deque<Event>::iterator eq_it = ready_queue.begin();
    while (eq_it != ready_queue.end() && temp.dynamic_p <= eq_it->dynamic_p) {
        eq_it++;
    }
    eq_it = ready_queue.insert(eq_it, temp);
    return;
}

void insert_expiredq(Event temp){
    temp.dynamic_p = temp.p.priority + 1;
    deque<Event>::iterator eq_it = expired_queue.begin();
    while (eq_it != expired_queue.end() && temp.dynamic_p <= eq_it->dynamic_p) {
        eq_it++;
    }
    eq_it = expired_queue.insert(eq_it, temp);
    return;
}

void final_calculate(){
    process_info.finish = current_t;
    process_info.io_util = 100 * ((double)process_info.finish - (double)no_it) / process_info.finish;
    process_info.cpu_util = 100 * (double)process_info.cpu_util / (double)process_info.finish;
    process_info.avg_turnaround = (double)process_info.avg_turnaround / (double)num_process;
    process_info.avg_waittime = (double)process_info.avg_waittime / (double)num_process;
    process_info.throughput = 100 * (double)num_process / (double)process_info.finish;
    return;
}

void print_info(){
    for (int i = 0; i < num_process; i++) {
        printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n",
               result_queue[i].p.id,
               result_queue[i].p.arrive_t,
               result_queue[i].p.total_cpu_t,
               result_queue[i].p.cpu_burst,
               result_queue[i].p.io_burst,
               result_queue[i].p.priority + 1,
               result_queue[i].finish_t,
               result_queue[i].finish_t - result_queue[i].p.arrive_t,
               result_queue[i].io_t,
               result_queue[i].ready_t);
    }
    
    printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",
           process_info.finish,
           process_info.cpu_util,
           process_info.io_util,
           process_info.avg_turnaround,
           process_info.avg_waittime,
           process_info.throughput);
}

class Scheduler{
public:
    void add_process() {
        pFile=fopen (inputfile,"r");
        if (pFile == NULL) {
            cout<< "Error opening file";
            exit(0);
        }
        
        process_info.cpu_util = 0; // store total cpu of all processes
        process_info.avg_turnaround = 0;
        
        read_char = fgetc(pFile);
        int transfer;
        stringstream ss; //container to transfer string to integer
        int process_id = 0;
        while (read_char != EOF) {
            Event temp;
            Process tmp;
            tmp.id = process_id;
            skip_delimiter();
            ss << read_int();//arrival time
            ss >> transfer;
            tmp.arrive_t = transfer;
            ss.clear();
            skip_delimiter();
            ss << read_int();//total cpu time
            ss >> transfer;
            tmp.total_cpu_t = transfer;
            process_info.cpu_util = process_info.cpu_util + transfer;
            ss.clear();
            skip_delimiter();
            ss << read_int();//cpu burst
            ss >> tmp.cpu_burst;
            ss.clear();
            skip_delimiter();
            ss << read_int(); //io burst
            ss >> tmp.io_burst;
            ss.clear();
            getline(rFile, read_rstring);//priority
            int static_p;
            ss << read_rstring;
            ss >> static_p;
            ss.clear();
            tmp.priority = static_p % 4;
            
            temp.p = tmp; //initialize event
            temp.state = "CREATED";
            temp.randval = 0;
            temp.time_stamp = temp.p.arrive_t;
            temp.io_t = 0;
            temp.ready_t = 0;
            temp.finish_t = 0;
            temp.rem = temp.p.total_cpu_t;
            temp.rem_cpu_burst = 0;
            temp.dynamic_p = tmp.priority + 1; // set dynamic priority
            global_queue.push_back(temp);
            
            process_id++;
            skip_delimiter();
        }
        num_process = process_id;
        fclose (pFile);
    }
    
    virtual void run_queue() = 0;
};

class FcfsScheduler : public Scheduler {
public:
    void run_queue() {
        while (!global_queue.empty()) {
            deque<Event>::iterator gl_it = global_queue.begin();
            Event first = *gl_it;
            no_it = no_it + first.time_stamp - current_t;
            first = bc_ready(first);
            ready_queue.push_back(first);
            gl_it = global_queue.erase(gl_it);
            while (gl_it != global_queue.end()) {
                if (first.p.arrive_t == gl_it->p.arrive_t) {
                    *gl_it = bc_ready(*gl_it);
                    ready_queue.push_back(*gl_it);
                    gl_it = global_queue.erase(gl_it);
                }
                else{
                    gl_it++;
                }
            }
            
            while (!event_queue.empty() || !ready_queue.empty()) {
                deque<Event>::iterator eq_it = event_queue.begin();
                deque<Event>::iterator rq_it = ready_queue.begin();
                Event temp;
                if (ready_queue.empty()) {
                    gl_it = global_queue.begin();
                    if (gl_it != global_queue.end() && gl_it->time_stamp <= eq_it->time_stamp) {
                        temp = *gl_it;
                        temp = bc_ready(temp);
                        gl_it = global_queue.erase(gl_it);
                        ready_queue.push_back(temp);
                    }
                    else{
                        temp = *eq_it;
                        temp = bc_ready(temp);
                        eq_it = event_queue.erase(eq_it);
                        ready_queue.push_back(temp);
                    }
                }
                else {
                    temp = *rq_it;
                    temp = ready_running(temp);
                    if (event_queue.empty()) {
                        no_it = no_it + temp.randval;
                    }
                    rq_it = ready_queue.erase(rq_it);
                    deque<Event>::iterator tr_it = event_queue.begin(); //put to ready queue
                    gl_it = global_queue.begin();
                    while ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) || (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                        if ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) && (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                            if (gl_it->time_stamp <= tr_it->time_stamp) {
                                Event to_ready = *gl_it;
                                to_ready = bc_ready(to_ready);
                                ready_queue.push_back(to_ready);
                                gl_it = global_queue.erase(gl_it);
                            }
                            else {
                                Event to_ready = *tr_it;
                                to_ready = bc_ready(to_ready);
                                ready_queue.push_back(to_ready);
                                tr_it = event_queue.erase(tr_it);
                                if (event_queue.empty()) {
                                    no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                                }
                            }
                        }
                        else if (tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) {
                            Event to_ready = *tr_it;
                            to_ready = bc_ready(to_ready);
                            ready_queue.push_back(to_ready);
                            tr_it = event_queue.erase(tr_it);
                            if (event_queue.empty()) {
                                no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                            }
                        }
                        else {
                            Event to_ready = *gl_it;
                            to_ready = bc_ready(to_ready);
                            ready_queue.push_back(to_ready);
                            gl_it = global_queue.erase(gl_it);
                        }
                    }
                    if (temp.rem == 0) {
                        temp = done(temp);
                        temp.turnaround_t = temp.finish_t - temp.p.arrive_t;
                        process_info.avg_turnaround = process_info.avg_turnaround + temp.turnaround_t;
                        process_info.avg_waittime = process_info.avg_waittime + temp.ready_t;
                        insert_rq(temp);
                    }
                    else{
                        temp = running_block(temp);
                        insert_eq(temp);
                    }
                }
            }
        }
        final_calculate();
        return;
    }
};

class LcfsScheduler: public Scheduler {
public:
    void run_queue() {
        while (!global_queue.empty()) {
            deque<Event>::iterator gl_it = global_queue.begin();
            Event first = *gl_it;
            no_it = no_it + first.time_stamp - current_t;
            first = bc_ready(first);
            ready_queue.push_front(first);
            gl_it = global_queue.erase(gl_it);
            while (gl_it != global_queue.end()) {
                if (first.p.arrive_t == gl_it->p.arrive_t) {
                    *gl_it = bc_ready(*gl_it);
                    ready_queue.push_front(*gl_it);
                    gl_it = global_queue.erase(gl_it);
                }
                else{
                    gl_it++;
                }
            }
            
            while (!event_queue.empty() || !ready_queue.empty()) {
                deque<Event>::iterator eq_it = event_queue.begin();
                deque<Event>::iterator rq_it = ready_queue.begin();
                Event temp;
                if (ready_queue.empty()) {
                    gl_it = global_queue.begin();
                    if (gl_it != global_queue.end() && gl_it->time_stamp <= eq_it->time_stamp) {
                        temp = *gl_it;
                        temp = bc_ready(temp);
                        gl_it = global_queue.erase(gl_it);
                        ready_queue.push_front(temp);
                    }
                    else{
                        temp = *eq_it;
                        temp = bc_ready(temp);
                        eq_it = event_queue.erase(eq_it);
                        ready_queue.push_front(temp);
                        while (eq_it != event_queue.end()) {
                            if (eq_it->time_stamp == temp.time_stamp) {
                                temp = *eq_it;
                                temp = bc_ready(temp);
                                eq_it = event_queue.erase(eq_it);
                                ready_queue.push_front(temp);
                            }
                            else{
                                break;
                            }
                        }
                        
                    }
                }
                else {
                    temp = *rq_it;
                    temp = ready_running(temp);
                    if (event_queue.empty()) {
                        no_it = no_it + temp.randval;
                    }
                    rq_it = ready_queue.erase(rq_it);
                    deque<Event>::iterator tr_it = event_queue.begin(); //put to ready queue
                    gl_it = global_queue.begin();
                    while ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) || (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                        if ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) && (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                            if (gl_it->time_stamp <= tr_it->time_stamp) {
                                Event to_ready = *gl_it;
                                to_ready = bc_ready(to_ready);
                                ready_queue.push_front(to_ready);
                                gl_it = global_queue.erase(gl_it);
                            }
                            else {
                                Event to_ready = *tr_it;
                                to_ready = bc_ready(to_ready);
                                ready_queue.push_front(to_ready);
                                tr_it = event_queue.erase(tr_it);
                                if (event_queue.empty()) {
                                    no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                                }
                            }
                        }
                        else if (tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) {
                            Event to_ready = *tr_it;
                            to_ready = bc_ready(to_ready);
                            ready_queue.push_front(to_ready);
                            tr_it = event_queue.erase(tr_it);
                            if (event_queue.empty()) {
                                no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                            }
                        }
                        else {
                            Event to_ready = *gl_it;
                            to_ready = bc_ready(to_ready);
                            ready_queue.push_front(to_ready);
                            gl_it = global_queue.erase(gl_it);
                        }
                    }
                    if (temp.rem == 0) {
                        temp = done(temp);
                        temp.turnaround_t = temp.finish_t - temp.p.arrive_t;
                        process_info.avg_turnaround = process_info.avg_turnaround + temp.turnaround_t;
                        process_info.avg_waittime = process_info.avg_waittime + temp.ready_t;
                        insert_rq(temp);
                    }
                    else{
                        temp = running_block(temp);
                        insert_eq(temp);
                    }
                }
            }
        }
        final_calculate();
        return;
    }
};

class SjfScheduler: public Scheduler {
public:
    void put_ready(Event temp){
        deque<Event>::iterator st_it = ready_queue.begin();
        while (st_it != ready_queue.end() && temp.rem >= st_it->rem) {
            if (temp.rem >= st_it->rem) {
                st_it++;
            }
            else{
                if (temp.randval < st_it->randval) {
                    st_it++;
                }
            }
        }
        st_it = ready_queue.insert(st_it, temp);
        return;
    }
    
    void run_queue() {
        while (!global_queue.empty()) {
            deque<Event>::iterator gl_it = global_queue.begin();
            Event first = *gl_it;
            no_it = no_it + first.time_stamp - current_t;
            first = bc_ready(first);
            put_ready(first);
            gl_it = global_queue.erase(gl_it);
            while (gl_it != global_queue.end()) {
                if (first.p.arrive_t == gl_it->p.arrive_t) {
                    *gl_it = bc_ready(*gl_it);
                    put_ready(*gl_it);
                    gl_it = global_queue.erase(gl_it);
                }
                else{
                    gl_it++;
                }
            }
            
            while (!event_queue.empty() || !ready_queue.empty()) {
                deque<Event>::iterator eq_it = event_queue.begin();
                deque<Event>::iterator rq_it = ready_queue.begin();
                Event temp;
                if (ready_queue.empty()) {
                    gl_it = global_queue.begin();
                    if (gl_it != global_queue.end() && gl_it->time_stamp <= eq_it->time_stamp) {
                        temp = *gl_it;
                        temp = bc_ready(temp);
                        gl_it = global_queue.erase(gl_it);
                        put_ready(temp);
                    }
                    else{
                        temp = *eq_it;
                        temp = bc_ready(temp);
                        eq_it = event_queue.erase(eq_it);
                        put_ready(temp);
                    }
                }
                else {
                    temp = *rq_it;
                    temp = ready_running(temp);
                    if (event_queue.empty()) {
                        no_it = no_it + temp.randval;
                    }
                    rq_it = ready_queue.erase(rq_it);
                    deque<Event>::iterator tr_it = event_queue.begin(); //put to ready queue
                    gl_it = global_queue.begin();
                    while ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) || (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                        if ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) && (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                            if (gl_it->time_stamp <= tr_it->time_stamp) {
                                Event to_ready = *gl_it;
                                to_ready = bc_ready(to_ready);
                                put_ready(to_ready);
                                gl_it = global_queue.erase(gl_it);
                            }
                            else {
                                Event to_ready = *tr_it;
                                to_ready = bc_ready(to_ready);
                                put_ready(to_ready);
                                tr_it = event_queue.erase(tr_it);
                                if (event_queue.empty()) {
                                    no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                                }
                            }
                        }
                        else if (tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) {
                            Event to_ready = *tr_it;
                            to_ready = bc_ready(to_ready);
                            put_ready(to_ready);
                            tr_it = event_queue.erase(tr_it);
                            if (event_queue.empty()) {
                                no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                            }
                        }
                        else {
                            Event to_ready = *gl_it;
                            to_ready = bc_ready(to_ready);
                            put_ready(to_ready);
                            gl_it = global_queue.erase(gl_it);
                        }
                    }
                    if (temp.rem == 0) {
                        temp = done(temp);
                        temp.turnaround_t = temp.finish_t - temp.p.arrive_t;
                        process_info.avg_turnaround = process_info.avg_turnaround + temp.turnaround_t;
                        process_info.avg_waittime = process_info.avg_waittime + temp.ready_t;
                        insert_rq(temp);
                    }
                    else{
                        temp = running_block(temp);
                        insert_eq(temp);
                    }
                }
            }
        }
        final_calculate();
        return;
    }
};

class RrScheduler: public Scheduler {
public:
    void run_queue() {
        while (!global_queue.empty()) {
            deque<Event>::iterator gl_it = global_queue.begin();
            Event first = *gl_it;
            no_it = no_it + first.time_stamp - current_t;
            first = bc_ready(first);
            ready_queue.push_back(first);
            gl_it = global_queue.erase(gl_it);
            while (gl_it != global_queue.end()) {
                if (first.p.arrive_t == gl_it->p.arrive_t) {
                    *gl_it = bc_ready(*gl_it);
                    ready_queue.push_back(*gl_it);
                    gl_it = global_queue.erase(gl_it);
                }
                else{
                    gl_it++;
                }
            }
            
            while (!event_queue.empty() || !ready_queue.empty()) {
                deque<Event>::iterator eq_it = event_queue.begin();
                deque<Event>::iterator rq_it = ready_queue.begin();
                Event temp;
                if (ready_queue.empty()) {
                    gl_it = global_queue.begin();
                    if (gl_it != global_queue.end() && gl_it->time_stamp <= eq_it->time_stamp) {
                        temp = *gl_it;
                        temp = bc_ready(temp);
                        gl_it = global_queue.erase(gl_it);
                        ready_queue.push_back(temp);
                    }
                    else{
                        temp = *eq_it;
                        temp = bc_ready(temp);
                        eq_it = event_queue.erase(eq_it);
                        ready_queue.push_back(temp);
                    }
                }
                else {
                    temp = *rq_it;
                    temp = run_quantum(temp);
                    if (event_queue.empty()) {
                        no_it = no_it + temp.randval;
                    }
                    rq_it = ready_queue.erase(rq_it);
                    deque<Event>::iterator tr_it = event_queue.begin(); //put to ready queue
                    gl_it = global_queue.begin();
                    while ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) || (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                        if ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) && (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                            if (gl_it->time_stamp <= tr_it->time_stamp) {
                                Event to_ready = *gl_it;
                                to_ready = bc_ready(to_ready);
                                ready_queue.push_back(to_ready);
                                gl_it = global_queue.erase(gl_it);
                            }
                            else {
                                Event to_ready = *tr_it;
                                to_ready = bc_ready(to_ready);
                                ready_queue.push_back(to_ready);
                                tr_it = event_queue.erase(tr_it);
                                if (event_queue.empty()) {
                                    no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                                }
                            }
                        }
                        else if (tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) {
                            Event to_ready = *tr_it;
                            to_ready = bc_ready(to_ready);
                            ready_queue.push_back(to_ready);
                            tr_it = event_queue.erase(tr_it);
                            if (event_queue.empty()) {
                                no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                            }
                        }
                        else {
                            Event to_ready = *gl_it;
                            to_ready = bc_ready(to_ready);
                            ready_queue.push_back(to_ready);
                            gl_it = global_queue.erase(gl_it);
                        }
                    }
                    if (temp.rem == 0) {
                        temp = done(temp);
                        temp.turnaround_t = temp.finish_t - temp.p.arrive_t;
                        process_info.avg_turnaround = process_info.avg_turnaround + temp.turnaround_t;
                        process_info.avg_waittime = process_info.avg_waittime + temp.ready_t;
                        insert_rq(temp);
                    }
                    else{
                        if (temp.rem_cpu_burst == 0) {
                            temp = running_block(temp);
                            insert_eq(temp);
                        }
                        else{
                            temp = running_ready(temp);
                            ready_queue.push_back(temp);
                        }
                    }
                }
            }
        }
        final_calculate();
        return;
    }
};

class PrioScheduler: public Scheduler {
public:
    void run_queue() {
        while (!global_queue.empty()) {
            deque<Event>::iterator gl_it = global_queue.begin();
            Event first = *gl_it;
            no_it = no_it + first.time_stamp - current_t;
            first.dynamic_p = first.dynamic_p - 1;
            first = bc_ready(first);
            insert_readyq(first);
            gl_it = global_queue.erase(gl_it);
            while (gl_it != global_queue.end()) {
                if (first.p.arrive_t == gl_it->p.arrive_t) {
                    gl_it->dynamic_p = gl_it->dynamic_p - 1;
                    *gl_it = bc_ready(*gl_it);
                    insert_readyq(*gl_it);
                    gl_it = global_queue.erase(gl_it);
                }
                else{
                    gl_it++;
                }
            }
            
            while (!event_queue.empty() || !ready_queue.empty() || !expired_queue.empty()) {
                deque<Event>::iterator eq_it = event_queue.begin();
                deque<Event>::iterator rq_it = ready_queue.begin();
                Event temp;
                if (ready_queue.empty()) {
                    if (!expired_queue.empty()) {
                        deque<Event>::iterator change_p = expired_queue.begin();
                        while (change_p != expired_queue.end()) {
                            change_p->dynamic_p = change_p->dynamic_p - 1;
                            change_p++;
                        }
                        swap(expired_queue, ready_queue);
                        rq_it = ready_queue.begin();
                        temp = *rq_it;
                        temp = run_quantum(temp);
                        if (event_queue.empty()) {
                            no_it = no_it + temp.randval;
                        }
                        rq_it = ready_queue.erase(rq_it);
                        deque<Event>::iterator tr_it = event_queue.begin(); //put to ready queue
                        gl_it = global_queue.begin();
                        while ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) || (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                            if ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) && (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                                if (gl_it->time_stamp <= tr_it->time_stamp) {
                                    Event to_ready = *gl_it;
                                    to_ready.dynamic_p = to_ready.dynamic_p - 1;
                                    to_ready = bc_ready(to_ready);
                                    insert_readyq(to_ready);
                                    gl_it = global_queue.erase(gl_it);
                                }
                                else {
                                    Event to_ready = *tr_it;
                                    to_ready.dynamic_p = to_ready.dynamic_p - 1;
                                    to_ready = bc_ready(to_ready);
                                    if (to_ready.dynamic_p < 0) {
                                        insert_expiredq(to_ready);
                                    }
                                    else{
                                        insert_readyq(to_ready);
                                    }
                                    tr_it = event_queue.erase(tr_it);
                                    if (event_queue.empty()) {
                                        no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                                    }
                                }
                            }
                            else if (tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) {
                                Event to_ready = *tr_it;
                                to_ready.dynamic_p = to_ready.dynamic_p - 1;
                                to_ready = bc_ready(to_ready);
                                if (to_ready.dynamic_p < 0) {
                                    insert_expiredq(to_ready);
                                }
                                else{
                                    insert_readyq(to_ready);
                                }
                                tr_it = event_queue.erase(tr_it);
                                if (event_queue.empty()) {
                                    no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                                }
                            }
                            else {
                                Event to_ready = *gl_it;
                                to_ready.dynamic_p = to_ready.dynamic_p - 1;
                                to_ready = bc_ready(to_ready);
                                insert_readyq(to_ready);
                                gl_it = global_queue.erase(gl_it);
                            }
                        }
                        if (temp.rem == 0) {
                            temp = done(temp);
                            temp.turnaround_t = temp.finish_t - temp.p.arrive_t;
                            process_info.avg_turnaround = process_info.avg_turnaround + temp.turnaround_t;
                            process_info.avg_waittime = process_info.avg_waittime + temp.ready_t;
                            insert_rq(temp);
                        }
                        else{
                            if (temp.rem_cpu_burst == 0) {
                                temp = running_block(temp);
                                temp.dynamic_p = temp.p.priority + 1;
                                insert_eq(temp);
                            }
                            else{
                                temp = running_readyp(temp);
                                temp.dynamic_p = temp.dynamic_p - 1;
                                if (temp.dynamic_p < 0) {
                                    insert_expiredq(temp);
                                }
                                else{
                                    insert_readyq(temp);
                                }
                            }
                        }
                        
                    }
                    // expired Q and ready Q both empty
                    else{
                        gl_it = global_queue.begin();
                        if (gl_it != global_queue.end() && gl_it->time_stamp <= eq_it->time_stamp) {
                            temp = *gl_it;
                            temp.dynamic_p = temp.dynamic_p - 1;
                            temp = bc_ready(temp);
                            gl_it = global_queue.erase(gl_it);
                            insert_readyq(temp);
                        }
                        else{
                            temp = *eq_it;
                            temp.dynamic_p = temp.dynamic_p - 1;
                            temp = bc_ready(temp);
                            eq_it = event_queue.erase(eq_it);
                            if (temp.dynamic_p < 0) {
                                insert_expiredq(temp);
                            }
                            else{
                                insert_readyq(temp);
                            }
                            deque<Event>::iterator it = event_queue.begin();
                            while (it != event_queue.end() && it->time_stamp == temp.time_stamp) {
                                Event temp2 = *it;
                                temp2.dynamic_p = temp2.dynamic_p - 1;
                                temp2 = bc_ready(temp2);
                                if (temp2.dynamic_p < 0) {
                                    insert_expiredq(temp2);
                                }
                                else{
                                    insert_readyq(temp2);
                                }
                                it = event_queue.erase(it);
                            }
                        }
                    }
                    
                }
                // ready queue not null
                else {
                    temp = *rq_it;
                    temp = run_quantum(temp);
                    if (event_queue.empty()) {
                        no_it = no_it + temp.randval;
                    }
                    rq_it = ready_queue.erase(rq_it);
                    deque<Event>::iterator tr_it = event_queue.begin(); //put to ready queue
                    gl_it = global_queue.begin();
                    while ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) || (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                        if ((tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) && (gl_it != global_queue.end() && gl_it->time_stamp <= temp.time_stamp)) {
                            if (gl_it->time_stamp <= tr_it->time_stamp) {
                                Event to_ready = *gl_it;
                                to_ready.dynamic_p = to_ready.dynamic_p - 1;
                                to_ready = bc_ready(to_ready);
                                insert_readyq(to_ready);
                                gl_it = global_queue.erase(gl_it);
                            }
                            else {
                                Event to_ready = *tr_it;
                                to_ready.dynamic_p = to_ready.dynamic_p - 1;
                                to_ready = bc_ready(to_ready);
                                if (to_ready.dynamic_p < 0) {
                                    insert_expiredq(to_ready);
                                }
                                else{
                                    insert_readyq(to_ready);
                                }
                                tr_it = event_queue.erase(tr_it);
                                if (event_queue.empty()) {
                                    no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                                }
                            }
                        }
                        else if (tr_it != event_queue.end() && tr_it->time_stamp <= temp.time_stamp) {
                            Event to_ready = *tr_it;
                            to_ready.dynamic_p = to_ready.dynamic_p - 1;
                            to_ready = bc_ready(to_ready);
                            if (to_ready.dynamic_p < 0) {
                                insert_expiredq(to_ready);
                            }
                            else{
                                insert_readyq(to_ready);
                            }
                            tr_it = event_queue.erase(tr_it);
                            if (event_queue.empty()) {
                                no_it = no_it + temp.time_stamp - tr_it->time_stamp;
                            }
                        }
                        else {
                            Event to_ready = *gl_it;
                            to_ready.dynamic_p = to_ready.dynamic_p - 1;
                            to_ready = bc_ready(to_ready);
                            insert_readyq(to_ready);
                            gl_it = global_queue.erase(gl_it);
                        }
                    }
                    if (temp.rem == 0) {
                        temp = done(temp);
                        temp.turnaround_t = temp.finish_t - temp.p.arrive_t;
                        process_info.avg_turnaround = process_info.avg_turnaround + temp.turnaround_t;
                        process_info.avg_waittime = process_info.avg_waittime + temp.ready_t;
                        insert_rq(temp);
                    }
                    else{
                        if (temp.rem_cpu_burst == 0) {
                            temp = running_block(temp);
                            temp.dynamic_p = temp.p.priority + 1;
                            insert_eq(temp);
                        }
                        else{
                            temp = running_readyp(temp);
                            temp.dynamic_p = temp.dynamic_p - 1;
                            if (temp.dynamic_p < 0) {
                                insert_expiredq(temp);
                            }
                            else{
                                insert_readyq(temp);
                            }
                        }
                    }
                }
            }
        }
        final_calculate();
        return;
    }
};

int main(int argc, char * argv[]) {
    Scheduler * test = NULL;
    
    char * input = argv[1];
    inputfile = argv[2];
    char * randfile = argv[3];
    
    string str(input);
    char spara = input[2];
    string q = "";
    if (str.length() >= 3) {
        for (int i = 3; i < str.length(); i++) {
            q = q + input[i];
        }
    }
    stringstream ss;
    ss << q;
    ss >> quantum;
    ss.clear();
    
    rFile.open(randfile);
    if (!rFile.is_open()) {
        cout<< "Error opening file" << endl;
        exit(0);
    }
    if (argc != 4) {
        cout<< "Wrong input arguments" << endl;
        exit(0);
     }
    
    getline(rFile, read_rstring);//filter the first line, which indicate the num of random num in rfile
    
     switch (spara) {
     case 'F':
     test = new FcfsScheduler();
     test->add_process();
     test->run_queue();
     cout<< "FCFS" << endl;
     print_info();
     break;
     case 'L':
     test = new LcfsScheduler();
     test->add_process();
     test->run_queue();
     cout<< "LCFS" << endl;
     print_info();
     break;
     case 'S':
     test = new SjfScheduler();
     test->add_process();
     test->run_queue();
     cout<< "SJF" << endl;
     print_info();
     break;
     case 'R':
     test = new RrScheduler();
     test->add_process();
     test->run_queue();
     cout<< "RR " << quantum << endl;
     print_info();
     break;
     case 'P':
     test = new PrioScheduler();
     test->add_process();
     test->run_queue();
     cout<< "PRIO " << quantum << endl;
     print_info();
     break;
     default:
     cout<< "No such scheduler";
     break;
     }
    
    rFile.close();//close rfile until the whole process ends
    return 0;
}
