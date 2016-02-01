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

ifstream pFile;

class IOScheduler {
public:
    int time_stamp;
    int num_operation;
    int pre_head;
    int head; //initially at track 0
    int direction; //0 represents to lower, 1 represents to higher
    operation pre_pick; //previous operation
    int flag; //1:ioqueue is processed, 2: ioqueue2 is processed
    summary sum;
    deque<operation> all_oper;
    deque<operation> ioqueue;
    deque<operation> resultq;
    deque<operation> ioqueue2;
    
    virtual operation pick_oper() = 0;
    
    void var_initial() {
        pre_pick.no = 0;
        flag = 1;
        direction = 1;
        pre_head = 0;
        head = 0;
        num_operation = -1;
        time_stamp = 0;
        
        sum.total_time = 0;
        sum.tot_movement = 0;
        sum.avg_turnaround = 0;
        sum.avg_waittime = 0;
        sum.max_waittime = 0;
        return;
    }
    
    void read_all_oper(){
        var_initial();
        string read_istring; //read instrutctions line by line from input file
        if (!pFile.is_open()) {
            cout<< "Error opening input file";
            exit(1);
        }
        stringstream ss;
        int temp_time;
        int temp_track;
        operation temp;
        temp.start_t = 0;
        temp.end_t = 0;
        getline(pFile, read_istring); // get first instr line
        while (!pFile.eof()) {
            num_operation++;
            while (read_istring.substr(0, 1) == "#" && !pFile.eof()) { // skip comments
                getline(pFile, read_istring);
            }
            for (int i = 0; i < read_istring.length(); i++) {
                if (read_istring.at(i) == ' ') {
                    ss<< read_istring.substr(0, i);
                    ss>> temp_time;
                    ss.clear();
                    ss<< read_istring.substr(i+1, read_istring.length());
                    ss>> temp_track;
                    ss.clear();
                    temp.no = num_operation;
                    temp.time = temp_time;
                    temp.track = temp_track;
                    all_oper.push_back(temp);
                }
            }
            getline(pFile, read_istring);
        }
        pFile.close();
        return;
    }
    
    void request_proccess(){
        while (!all_oper.empty()) {
            deque<operation>::iterator it = all_oper.begin();
            operation temp = *it;
            time_stamp = temp.time;
            all_oper.pop_front();
            ioqueue.push_back(temp);
            while (!ioqueue.empty()) {
                operation pick = pick_oper(); //add
                pre_head = head;
                head = pick.track;
                pick.end_t = abs(pick.track - pre_head) + time_stamp;
                sum.tot_movement = abs(pick.track - pre_head) + sum.tot_movement;
                
                pick.start_t = time_stamp;// issue
                sum.avg_waittime = sum.avg_waittime + pick.start_t - pick.time;
                if (sum.max_waittime < pick.start_t - pick.time) {
                    sum.max_waittime = pick.start_t - pick.time;
                }
                
                deque<operation>::iterator it_all = all_oper.begin();
                while (!all_oper.empty() && it_all->time <= pick.end_t) {
                    operation qualify = *it_all;
                    all_oper.pop_front();
                    ioqueue.push_back(qualify);
                    it_all++;
                }
                
                time_stamp = pick.end_t;//finish -pop in callee
                pre_pick = pick;
                resultq.push_back(pick);
                sum.avg_turnaround = sum.avg_turnaround + pick.end_t - pick.time;
            }
        }
        return;
    }
    
    
    void request_p(){
        while (!all_oper.empty()) {
            deque<operation>::iterator it = all_oper.begin();
            operation temp = *it;
            time_stamp = temp.time;
            all_oper.pop_front();
            ioqueue.push_back(temp);
            flag = 1;
            while (!ioqueue.empty() || !ioqueue2.empty()) {
                if (ioqueue.empty()) {
                    direction = 1;
                    flag = 2;
                }
                if (ioqueue2.empty()) {
                    direction = 1;
                    flag = 1;
                }
                operation pick = pick_oper(); //add
                pre_head = head;
                head = pick.track;
                pick.end_t = abs(pick.track - pre_head) + time_stamp;
                sum.tot_movement = abs(pick.track - pre_head) + sum.tot_movement;
                
                pick.start_t = time_stamp;// issue
                sum.avg_waittime = sum.avg_waittime + pick.start_t - pick.time;
                if (sum.max_waittime < pick.start_t - pick.time) {
                    sum.max_waittime = pick.start_t - pick.time;
                }
                
                deque<operation>::iterator it_all = all_oper.begin();
                if (flag == 1) {
                    while (!all_oper.empty() && it_all->time <= pick.end_t) {
                        operation qualify = *it_all;
                        all_oper.pop_front();
                        ioqueue2.push_back(qualify);
                        it_all++;
                    }
                }
                else { //flag = 2
                    while (!all_oper.empty() && it_all->time <= pick.end_t) {
                        operation qualify = *it_all;
                        all_oper.pop_front();
                        ioqueue.push_back(qualify);
                        it_all++;
                    }
                }
                
                time_stamp = pick.end_t;//finish -pop in callee
                pre_pick = pick;
                resultq.push_back(pick);
                sum.avg_turnaround = sum.avg_turnaround + pick.end_t - pick.time;
            }
        }
        return;
    }
    
    void print_sum() {
        sum.total_time = resultq[num_operation].end_t;
        sum.avg_turnaround = sum.avg_turnaround / (num_operation + 1);
        sum.avg_waittime = sum.avg_waittime / (num_operation + 1);
        printf("SUM: %d %d %.2lf %.2lf %d\n", sum.total_time,
               sum.tot_movement, sum.avg_turnaround, sum.avg_waittime, sum.max_waittime);
    }
};


class FIFOScheduler: public IOScheduler {
public:
    operation pick_oper() {
        deque<operation>::iterator it = ioqueue.begin();
        operation temp = *it;
        ioqueue.pop_front();
        return temp;
    }
};


class SSTFScheduler: public IOScheduler {
public:
    operation pick_oper() {
        deque<operation>::iterator it = ioqueue.begin();
        operation temp = *it;
        int min_d = abs(ioqueue.front().track - pre_pick.track);
        int min_p = 0;
        int i = -1;
        while (!ioqueue.empty() && it != ioqueue.end()) {
            i++;
            if (abs(pre_pick.track - it->track) < min_d) {
                min_d = abs(pre_pick.track - it->track);
                min_p = i;
            }
            it++;
        }
        temp = ioqueue[min_p];
        ioqueue.erase(ioqueue.begin() + min_p);
        return temp;
    }
};


class SCANScheduler: public IOScheduler {
public:
    operation pick_oper() {
        deque<operation>::iterator it = ioqueue.begin();
        operation temp = ioqueue.front();
        int min_track = pre_pick.track;
        int max_track = pre_pick.track;
        int min_d = 999999; //check range
        int min_p = 0;
        int i = 0;
        while (!ioqueue.empty() && it != ioqueue.end()) {
            if (it->track == pre_pick.track) {  //////////
                temp = *it;
                ioqueue.erase(it);
                return temp;
            }
            if (it->track < min_track) {
                min_track = it->track;
            }
            if (it->track > max_track) {
                max_track = it->track;
            }
            it++;
        }
        if (pre_pick.track == max_track) {
            direction = 0;
        }
        if (pre_pick.track == min_track) {
            direction = 1;
        }
        it = ioqueue.begin();
        while (!ioqueue.empty() && it != ioqueue.end()) {
            if (direction == 1 && it->track >= pre_pick.track) {
                if (abs(pre_pick.track - it->track) < min_d) {
                    min_d = abs(pre_pick.track - it->track);
                    min_p = i;
                }
            }
            if (direction == 0 && it->track <= pre_pick.track) {
                if (abs(pre_pick.track - it->track) < min_d) {
                    min_d = abs(pre_pick.track - it->track);
                    min_p = i;
                }
            }
            it++;
            i++;
        }
        temp = ioqueue[min_p];
        ioqueue.erase(ioqueue.begin() + min_p);
        return temp;
    }
};


class CSCANScheduler: public IOScheduler {
public:
    operation pick_oper() {
        deque<operation>::iterator it = ioqueue.begin();
        operation temp = ioqueue.front();
        int min_track = pre_pick.track;
        int max_track = pre_pick.track;
        int min_d = 999999; //check range
        int min_p = 0;
        int i = 0;
        while (!ioqueue.empty() && it != ioqueue.end()) {
            if (it->track == pre_pick.track) {
                temp = *it;
                ioqueue.erase(it);
                return temp;
            }
            if (it->track < min_track) {
                min_track = it->track;
                min_p = i;
            }
            if (it->track > max_track) {
                max_track = it->track;
            }
            it++;
            i++;
        }
        if (pre_pick.track == max_track) {
            temp = ioqueue[min_p];
            ioqueue.erase(ioqueue.begin() + min_p);
            return temp;
        }
        else {
            i = 0;
            it = ioqueue.begin();
            while (!ioqueue.empty() && it != ioqueue.end()) {
                if (abs(pre_pick.track - it->track) < min_d && it->track >= pre_pick.track) {
                    min_d = abs(pre_pick.track - it->track);
                    min_p = i;
                }
                it++;
                i++;
            }
            temp = ioqueue[min_p];
            ioqueue.erase(ioqueue.begin() + min_p);
            return temp;
        }
    }
};


class FSCANScheduler: public IOScheduler {
public:
    operation pick_oper() {
        deque<operation>::iterator it;
        operation temp;
        int min_track = pre_pick.track;
        int max_track = pre_pick.track;
        int min_d = 999999; //check range
        int min_p = 0;
        int i = 0;
        
        if (flag == 1) {
            it = ioqueue.begin();
            temp = ioqueue.front();
            while (!ioqueue.empty() && it != ioqueue.end()) {
                if (it->track == pre_pick.track) { ////////////
                    temp = *it;
                    ioqueue.erase(it);
                    return temp;
                }
                if (it->track < min_track) {
                    min_track = it->track;
                }
                if (it->track > max_track) {
                    max_track = it->track;
                }
                it++;
            }
            if (pre_pick.track == max_track) {
                direction = 0;
            }
            if (pre_pick.track == min_track) {
                direction = 1;
            }
            it = ioqueue.begin();
            while (!ioqueue.empty() && it != ioqueue.end()) {
                if (direction == 1 && it->track >= pre_pick.track) {
                    if (abs(pre_pick.track - it->track) < min_d) {
                        min_d = abs(pre_pick.track - it->track);
                        min_p = i;
                    }
                }
                if (direction == 0 && it->track <= pre_pick.track) {
                    if (abs(pre_pick.track - it->track) < min_d) {
                        min_d = abs(pre_pick.track - it->track);
                        min_p = i;
                    }
                }
                it++;
                i++;
            }
            temp = ioqueue[min_p];
            ioqueue.erase(ioqueue.begin() + min_p);
        }
        else { // flag = 2
            it = ioqueue2.begin();
            temp = ioqueue2.front();
            while (!ioqueue2.empty() && it != ioqueue2.end()) {
                if (it->track == pre_pick.track) { ///////////
                    temp = *it;
                    ioqueue2.erase(it);
                    return temp;
                }
                if (it->track < min_track) {
                    min_track = it->track;
                }
                if (it->track > max_track) {
                    max_track = it->track;
                }
                it++;
            }
            if (pre_pick.track == max_track) {
                direction = 0;
            }
            if (pre_pick.track == min_track) {
                direction = 1;
            }
            it = ioqueue2.begin();
            while (!ioqueue2.empty() && it != ioqueue2.end()) {
                if (direction == 1 && it->track >= pre_pick.track) {
                    if (abs(pre_pick.track - it->track) < min_d) {
                        min_d = abs(pre_pick.track - it->track);
                        min_p = i;
                    }
                }
                if (direction == 0 && it->track <= pre_pick.track) {
                    if (abs(pre_pick.track - it->track) < min_d) {
                        min_d = abs(pre_pick.track - it->track);
                        min_p = i;
                    }
                }
                it++;
                i++;
            }
            temp = ioqueue2[min_p];
            ioqueue2.erase(ioqueue2.begin() + min_p);
        }
        return temp;
    }
    
    
};

int main(int argc, char * argv[]) {
    IOScheduler *test = NULL;
    
    char * input = argv[2];
    pFile.open(input);
    
    char * para = argv[1];
    char spara = para[2];
    
    switch (spara) {
        case 'i':
            test = new FIFOScheduler();
            test->read_all_oper();
            test->request_proccess();
            break;
        case 'j':
            test = new SSTFScheduler();
            test->read_all_oper();
            test->request_proccess();
            break;
        case 's':
            test = new SCANScheduler();
            test->read_all_oper();
            test->request_proccess();
            break;
        case 'c':
            test = new CSCANScheduler();
            test->read_all_oper();
            test->request_proccess();
            break;
        case 'f':
            test = new FSCANScheduler();
            test->read_all_oper();
            test->request_p();
            break;
        default:
            cout << "No such IOScheduler" << endl;
            break;
    }
    test->print_sum();

    return 0;
}

