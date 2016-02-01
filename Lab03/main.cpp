#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <deque>
#include <bitset>
#include "definition.cpp"

const int VP_NUM = 64;

using namespace std;

ifstream pFile;
ifstream rFile;
string read_istring; //read instrutctions line by line from input file

pte v_table [64]; //declare virtual page table
deque<freelist> f_table;
counter stats;
int f_num = 1; //store para of frame table
int rlength = 10000;
int rarray[10000];
int rpointer = 0;
flaggroup flaggroup;

class PageReplace {
public:
    int threshold; //invoke potential page fault when threshold >= f_num
    int instr_type;
    int instr_vp;
    int oldest_p;
    int replace_count;
    bitset<32> bitgroupv[64];
    deque< bitset<32> > bitgroupp;
    
    virtual int select_victim() = 0;
    
    virtual void reshuffle(int vp) = 0;
    
    int myrandom(){
        int temp;
        if (rpointer == 10000) {
            rpointer = 0;
        }
        temp = rarray[rpointer];
        rpointer++;
        return temp % f_num;
    }
    
    int random(int size){
        int temp;
        if (rpointer == 10000) {
            rpointer = 0;
        }
        temp = rarray[rpointer];
        rpointer++;
        return temp % size;
    }
    
    void array_initialization(){
        if (!rFile.is_open()) {
            cout<< "Error opening rfile";
            exit(1);
        }
        string read_rstring;
        int temp;
        stringstream ss;
        getline(rFile, read_rstring);
        for (int i = 0; i < rlength; i++) {
            getline(rFile, read_rstring);
            ss << read_rstring;
            ss >> temp;
            ss.clear();
            rarray[i] = temp;
        }
        return;
    }
    
    void initialization(){
        threshold = 0; //invoke potential page fault when threshold >= f_num
        instr_type = 0;
        instr_vp = 0;
        oldest_p = 0;
        replace_count = 0;
        for (int i = 0; i < VP_NUM; i++) { //initialize virtual page
            v_table[i].present = 0;
            v_table[i].modified = 0;
            v_table[i].referenced = 0;
            v_table[i].pageout = 0;
            v_table[i].reserved = 0;
            v_table[i].index = 0;
            bitgroupv[i] = 0;
        }
        for (int i = 0; i < f_num; i++) { //initialize frame table
            freelist temp;
            temp.vp = -1;
            temp.no = i;
            f_table.push_back(temp);
            bitgroupp.push_back(0); // ??? need to be initialized
        }
        stats.unmap = 0; //initialize counter
        stats.map = 0;
        stats.pagein = 0;
        stats.pageout = 0;
        stats.zero = 0;
        stats.instr = -1;
        array_initialization();
        return;
    }
    
    
    bool read_per_instr(){
        stringstream ss;
        
        getline(pFile, read_istring); // get first instr line
        if (!pFile.eof()) {
            while (read_istring.substr(0, 1) == "#" && !pFile.eof()) { // skip comments
                getline(pFile, read_istring);
            }
            if (pFile.eof() || read_istring.length() < 3) {
                return false;
            }
            else {
                ss << read_istring.substr(0, 1);
                ss >> instr_type;
                ss.clear();
                ss << read_istring.substr(2, 2);
                ss >> instr_vp;
                ss.clear();
                if (flaggroup.flago == true) {
                    cout << "==> inst: " << instr_type << " " << instr_vp << endl;
                }
            }
        }
        return true;
    }
    
    void replace(){
        initialization();
        while (!pFile.eof()) {
            stats.instr++;
            if (read_per_instr()) {
                if (threshold < f_num) { // free list available
                    if (v_table[instr_vp].present == 0) {
                        if (v_table[instr_vp].present == 0) {
                            if (flaggroup.flago == true) {
                                printf("%ld: ZERO      %3d\n", stats.instr, threshold);
                                printf("%ld: MAP   %3d %3d\n", stats.instr, instr_vp, threshold);
                            }
                            stats.zero++;
                            stats.map++;
                            v_table[instr_vp].present = 1;
                            v_table[instr_vp].referenced = 1;
                            v_table[instr_vp].modified = instr_type;
                            v_table[instr_vp].index = threshold;
                            f_table[threshold].vp = instr_vp;
                            threshold++;
                        }
                    }
                    else { //already present with free list available
                        if (v_table[instr_vp].modified == 0) {
                            v_table[instr_vp].modified = instr_type;
                        }
                        reshuffle(instr_vp);
                    }
                }
                else { //no free list
                    if (v_table[instr_vp].present == 0) { //page fault occurs
                        int victim = select_victim(); //victim = freelist.no
                        int i;
                        freelist temp;
                        for (i = 0; i < f_num; i++) {
                            if (f_table[i].no == victim) {
                                temp = f_table[i];
                                break;
                            }
                        }
                        if (flaggroup.flago == true) {
                            printf("%ld: UNMAP %3d %3d\n", stats.instr, temp.vp, victim);
                        }
                        stats.unmap++;
                        v_table[temp.vp].present = 0;
                        //v_table[temp.vp].referenced = 0;
                        if (v_table[temp.vp].modified == 1) {
                            if (flaggroup.flago == true) {
                                printf("%ld: OUT   %3d %3d\n", stats.instr, temp.vp, victim);
                            }
                            v_table[temp.vp].pageout = 1;
                            v_table[temp.vp].modified = 0;
                            stats.pageout++;
                        }
                        
                        if (v_table[instr_vp].pageout == 1) {
                            if (flaggroup.flago == true) {
                                printf("%ld: IN    %3d %3d\n", stats.instr, instr_vp, victim);
                            }
                            stats.pagein++;
                        }
                        else {
                            if (flaggroup.flago == true) {
                                printf("%ld: ZERO      %3d\n", stats.instr, victim);
                            }
                            stats.zero++;
                        }
                        if (flaggroup.flago == true) {
                            printf("%ld: MAP   %3d %3d\n", stats.instr, instr_vp, victim);
                        }
                        stats.map++;
                        v_table[instr_vp].present = 1;
                        v_table[instr_vp].referenced = 1;
                        v_table[instr_vp].modified = instr_type;
                        v_table[instr_vp].index = f_table[i].no;
                        f_table[i].vp = instr_vp;
                    }
                    else { //page present in the f_table
                        if (v_table[instr_vp].modified == 0) {
                            v_table[instr_vp].modified = instr_type;
                        }
                        v_table[instr_vp].referenced = 1;
                        reshuffle(instr_vp);
                    }
                } //process instr
            } //end read instr
        } //end of file
        return;
    }
    
    void print_vtable(){
        for (int i = 0; i < VP_NUM; i++) { //print v_table
            if (v_table[i].present == 0) {
                if (v_table[i].pageout == 1) {
                    printf("# ");
                }
                else {
                    printf("* ");
                }
            }
            else { //present = 1
                if (v_table[i].referenced == 0) {
                    printf("%d:-", i);
                }
                else {
                    printf("%d:R", i);
                }
                if (v_table[i].modified == 0) {
                    printf("-");
                }
                else {
                    printf("M");
                }
                if (v_table[i].pageout == 0) {
                    printf("- ");
                }
                else {
                    printf("S ");
                }
            }
        }
        printf("\n");
        return;
    }
    
    void print_ftable(){
        for (int i = 0; i < f_num; i++) {
            for (int j = 0; j < f_num; j++) {
                if (f_table[j].no == i) {
                    if (f_table[i].vp < 0) {
                        printf("* ");
                    }
                    else {
                        printf("%d ", f_table[j].vp);
                    }
                    break;
                }
            }
        }
        printf("\n");
        return;
    }
    
    void print_counter(){
        unsigned long int total = 400 * (stats.unmap + stats.map) + 3000 * (stats.pageout + stats.pagein) + stats.zero * 150 + stats.instr;
        printf("SUM %ld U=%ld M=%ld I=%ld O=%ld Z=%ld ===> %ld\n",
               stats.instr,
               stats.unmap,
               stats.map,
               stats.pagein,
               stats.pageout,
               stats.zero,
               total
               );
        return;
    }
   
};


class RandomPageReplace: public PageReplace {
public:
    int select_victim(){
        int victim = myrandom();
        return victim;
    }
    void reshuffle(int vp){
        return;
    }
};

class FifoPageReplace: public PageReplace {
public:
    int select_victim(){
        freelist temp = f_table[0];
        f_table.erase(f_table.begin());
        f_table.push_back(temp);
        return temp.no;
    }
    
    void reshuffle(int vp){
        return;
    }
};

class SecondPageReplace: public PageReplace {
public:
    int select_victim(){
        while (true) {
            if (v_table[f_table[oldest_p].vp].referenced == 1) {
                v_table[f_table[oldest_p].vp].referenced = 0;
                oldest_p++;
                if (oldest_p == f_num) {
                    oldest_p = 0;
                }
            }
            else {
                oldest_p++;
                if (oldest_p == f_num) {
                    oldest_p = 0;
                }
                break;
            }
        }
        if (oldest_p == 0) {
            return f_table[f_num-1].no;
        }
        else {
            return f_table[oldest_p-1].no;
        }
    }
    
    void reshuffle(int vp){
        return;
    }
};


class LruPageReplace: public PageReplace {
public:
    int select_victim(){
        freelist temp = f_table[0];
        f_table.erase(f_table.begin());
        f_table.push_back(temp);
        return temp.no;
        return temp.no;
    }
    
    void reshuffle(int vp){
        freelist temp;
        int i;
        if (threshold < f_num) {
            for (i = 0; i <= threshold; i++) {
                if (f_table[i].vp == vp) {
                    temp = f_table[i];
                    break;
                }
            }
            f_table.erase(f_table.begin()+i);
            f_table.insert(f_table.begin()+threshold-1, temp);
        }
        else {
            for (i = 0; i < f_num; i++) {
                if (f_table[i].vp == vp) {
                    temp = f_table[i];
                    break;
                }
            }
            f_table.erase(f_table.begin()+i);
            f_table.push_back(temp);
        }
        return;
    }
};


class CClockPageReplace: public PageReplace {
public:
    int select_victim(){
        while (true) {
            if (v_table[f_table[oldest_p].vp].referenced == 1) {
                v_table[f_table[oldest_p].vp].referenced = 0;
                oldest_p++;
                if (oldest_p == f_num) {
                    oldest_p = 0;
                }
            }
            else {
                oldest_p++;
                if (oldest_p == f_num) {
                    oldest_p = 0;
                }
                break;
            }
        }
        if (oldest_p == 0) {
            return f_table[f_num-1].no;
        }
        else {
            return f_table[oldest_p-1].no;
        }
    }
    
    void reshuffle(int vp){
        return;
    }
};


class XClockPageReplace: public PageReplace {
public:
    int select_victim(){
        while (true) {
            if (v_table[oldest_p].present == 1) {
                if (v_table[oldest_p].referenced == 1) {
                    v_table[oldest_p].referenced = 0;
                    oldest_p++;
                    if (oldest_p == 64) {
                        oldest_p = 0;
                    }
                }
                else {
                    oldest_p++;
                    if (oldest_p == 64) {
                        oldest_p = 0;
                    }
                    break;
                }
            }
            else {
                oldest_p++;
                if (oldest_p == 64) {
                    oldest_p = 0;
                }
            }
        }
        if (oldest_p == 0) {
            return v_table[63].index;
        }
        else {
            return v_table[oldest_p-1].index;
        }
    }
    
    void reshuffle(int vp){
        return;
    }
};


class NruPageReplace: public PageReplace {
public:
    void reset(){
        if (replace_count == 10) {
            replace_count = 0;
            for (int j = 0; j < 64; j++) {
                if (v_table[j].present == 1) {
                    v_table[j].referenced = 0;
                }
            }
        }
        return;
    }
    
    int select_victim(){
        replace_count++;
        vector<pte> class0;
        vector<pte> class1;
        vector<pte> class2;
        vector<pte> class3;
        for (int i = 0; i < VP_NUM; i++) {
            if (v_table[i].present == 1) {
                if (v_table[i].referenced == 0 && v_table[i].modified == 0) {
                    class0.push_back(v_table[i]);
                }
                if (v_table[i].referenced == 0 && v_table[i].modified == 1) {
                    class1.push_back(v_table[i]);
                }
                if (v_table[i].referenced == 1 && v_table[i].modified == 0) {
                    class2.push_back(v_table[i]);
                }
                else { //v_table[i].referenced = 1 && v_table[i].modified = 0
                    class3.push_back(v_table[i]);
                }
            }
        }
        int vp = 0;
        if (!class0.empty()) {
            vp = random((int)class0.size());
            reset();
            return class0[vp].index;
        }
        else if (!class1.empty()) {
            vp = random((int)class1.size());
            reset();
            return class1[vp].index;
        }
        else if (!class2.empty()) {
            vp = random((int)class2.size());
            reset();
            return class2[vp].index;
        }
        else { //!class3.empty
            vp = random((int)class3.size());
            reset();
            return class3[vp].index;
        }
    }
    
    void reshuffle(int vp){
        return;
    }
};


class YAgingPageReplace: public PageReplace {
public:
    
    int select_victim(){
        int vp = 0;
        int i;
        for (i = 0; i < VP_NUM; i++) {
            if (v_table[i].present == 1) {
                bitgroupv[i] >>= 1;
                if (v_table[i].referenced == 1) {
                    v_table[i].referenced = 0;
                    bitgroupv[i].set(31, 1);
                }
            }
        }
        for (i = 0; i < VP_NUM; i++) { //find the first present vp
            if (v_table[i].present == 1) {
                vp = i;
                break;
            }
        }
        for (; i < VP_NUM; i++) {
            if (v_table[i].present == 1) {
                if (bitgroupv[i].to_ulong() < bitgroupv[vp].to_ulong()) {
                    vp = i;
                }
            }
        }
        bitgroupv[vp].reset();
        return v_table[vp].index;
    }
    
    void reshuffle(int vp){
        return;
    }
};


class AgingPageReplace: public PageReplace {
public:
    int select_victim(){
        int fp;
        int i;
        for (i = 0; i < f_num; i++) {
            bitgroupp[i] >>= 1;
            if (v_table[f_table[i].vp].referenced == 1) {
                v_table[f_table[i].vp].referenced = 0;
                bitgroupp[i].set(31, 1);
            }
        }
        fp = 0;
        for (i = 1; i < f_num; i++) {
            if (bitgroupp[i].to_ulong() < bitgroupp[fp].to_ulong()) {
                fp = i;
            }
        }
        bitgroupp[fp].reset();
        return fp;
    }
    
    void reshuffle(int vp){
        return;
    }
};


int main(int argc, char * argv[]) {
    char * algo = argv[1];//-a option;
    char spara = algo[2];
    string options = argv[2]; //-o options
    string fnum = argv[3];
    
    stringstream ss;
    ss << fnum.substr(2, 2);
    ss >> f_num;
    char * input = argv[4];
    char * randfile = argv[5];
    rFile.open(randfile);
    pFile.open(input);
    if (!pFile.is_open()) {
        cout<< "Error opening input file";
        exit(1);
    }
    
    flaggroup.flago = false;//initialize flag group -OFPS
    flaggroup.flagf = false;
    flaggroup.flagp = false;
    flaggroup.flags = false;
    for (int i = 2; i < options.length(); i++) {
        if (options.at(i) == 'O') {
            flaggroup.flago = true;
        }
        if (options.at(i) == 'P') {
            flaggroup.flagp = true;
        }
        if (options.at(i) == 'F') {
            flaggroup.flagf = true;
        }
        if (options.at(i) == 'S') {
            flaggroup.flags = true;
        }
    }
    
    PageReplace *instance = NULL;
    
    switch (spara) {
        case 'l':
            instance = new LruPageReplace();
            break;
        case 'r':
            instance = new RandomPageReplace();
            break;
        case 'f':
            instance = new FifoPageReplace();
            break;
        case 's':
            instance = new SecondPageReplace();
            break;
        case 'c':
            instance = new CClockPageReplace();
            break;
        case 'a':
            instance = new AgingPageReplace();
            break;
        case 'N':
            instance = new NruPageReplace();
            break;
        case 'X':
            instance = new XClockPageReplace();
            break;
        case 'Y':
            instance = new YAgingPageReplace();
            break;
        default:
            cout<< "No such algorithm" <<endl;
            break;
    }
    
    instance->replace();
    
    if (flaggroup.flagp == true) {
        instance->print_vtable();
    }
    if (flaggroup.flagf == true) {
        instance->print_ftable();
    }
    if (flaggroup.flags == true) {
        instance->print_counter();
    }

    pFile.close();
    rFile.close();
    
    return 0;
}

