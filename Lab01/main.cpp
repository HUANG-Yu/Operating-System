#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <list>
#include <vector>

using namespace std;

const int MAX_ADDR = 511;

FILE * pFile;
map<string, int> symbol_table; //store all the symbol in the input file
char read_char;
list<string> multi_def; //store multi-defined symbols
list<int> module_def_num; //store the number of symbol definition in each module
list<int> actual_mdef_num; //store the actual number of symbol def in each module, exluding those multi-defined
list<int> module_length; //store the length of each module
int module_num = 0; //remember how many modules in the input file
int ab_addr = 0; //remember the absolute address up till the current module
list<string> used_symbol; //store which symbol has been used in the uselist of all modules


int cline = 1; //remember current line number
int cl_offset = 1; //remember current line offset number
int pline = 1; //remember previous line number
int pl_offset = 1; //remember previous line number

void parse_error(int errcode, int errtype){//errcode represents error type, errtype represents which line & offset to be used
    string errstr[] = {
        "NUM_EXPECTED", // 0
        "SYM_EXPECTED", // 1
        "ADDR_EXPECTED", // 2
        "SYM_TOLONG", // 3
        "TO_MANY_DEF_IN_MODULE", // 4
        "TO_MANY_USE_IN_MODULE", // 5
        "TO_MANY_INSTR", // 6
    };
    if (errtype == 0) //errtype: 0 represents pline, 1/others represent cline
        cout << "Parse Error line " << pline << " offset " << pl_offset << ": " << errstr[errcode] << endl;
    else
        cout << "Parse Error line " << cline << " offset " << cl_offset << ": " << errstr[errcode] << endl;
    exit(0); //prompt to exit
}

void cal_error(int errcode){
    string errstr[] = {
        "Error: Absolute address exceeds machine size; zero used",//0
        "Error: Relative address exceeds module size; zero used",//1
        "Error: External address exceeds length of uselist; treated as immediate",//2
        "Error: Illegal immediate value; treated as 9999",//3
        "Error: Illegal opcode; treated as 9999",//4
        //symbol not defined; zero used directly wrap in the cal_usels_instrls() functions
    };
    cout<< errstr[errcode] << endl;
    return;
}


void skip_delimiter(){//skip token delimiters
    while (read_char == ' ' || read_char == '\n'||read_char == '\t') {
        pline = cline;
        pl_offset = cl_offset;
        cl_offset++;
        if (read_char == '\n') {
            cline++;
            cl_offset = 1;
        }
        read_char = fgetc(pFile);
    }
    return;
}


string read_symbol(){//read symbol by concatenating each legal character
    string temps = "";
    pline = cline;
    pl_offset = cl_offset;
    if (isalpha(read_char)) {//first char need to be character
        temps = temps + read_char;
        read_char = fgetc(pFile);
        cl_offset++;
    }
    else{
        parse_error(1, 1);
    }
    
    while (read_char != ' ' && read_char != '\n' && read_char != '\t' && read_char != EOF) {
        if(isalnum(read_char)){
            temps = temps + read_char;
            cl_offset++;
        }
        else{
            parse_error(1, 0);
        }
        read_char = fgetc(pFile);
        cl_offset++;
    }
    return temps;
}

string read_int(){//read symbol relative address by concatenating each legal character
    string temp = "";
    pline = cline;
    pl_offset = cl_offset;
    while (read_char != ' ' && read_char != '\n' && read_char != '\t' && read_char != EOF) {
        if(isdigit(read_char)){
            temp = temp + read_char;
        }
        else{
            parse_error(0, 0);
        }
        read_char = fgetc(pFile);
        cl_offset++;
    }
    return temp;
}


char read_type(){ //read the type of the program text
    char temps;
    pline = cline;
    pl_offset = cl_offset;
    if (read_char != 'I' && read_char != 'A' && read_char != 'R' && read_char != 'E') {
        parse_error(2, 1);
    }
    temps = read_char;
    read_char = fgetc(pFile);
    cl_offset++;
    if (read_char == EOF) {
        parse_error(2, 1);
    }
    if (read_char != ' ' && read_char != '\t' && read_char != '\n') {
        parse_error(2, 1);
    }
    return temps;
}


string read_addr(){//read symbol relative address by concatenating each legal character
    string temp = "";
    while (read_char == ' ' || read_char == '\n'||read_char == '\t') {
        cl_offset++;
        if (read_char == '\n') {
            cline++;
            cl_offset = 1;
        }
        read_char = fgetc(pFile);
    }
    if (read_char == EOF) {
        parse_error(2, 0);
    }
    while (read_char != ' ' && read_char != '\n' && read_char != '\t' && read_char != EOF) {
        if(isdigit(read_char)){
            temp = temp + read_char;
        }
        else{
            parse_error(2, 0);
        }
        read_char = fgetc(pFile);
        cl_offset++;
    }
    return temp;
}

void read_instr(){//read address in instructions of the program text
    string type;
    string instruction;
    
    if (read_char == EOF) {
        parse_error(2, 0);
    }
    skip_delimiter();
    if (read_char == EOF) {
        parse_error(2, 0);
    }
    type = read_type();
    if (read_char == EOF) {
        parse_error(2, 0);
    }
    instruction = read_addr();
    return;
}

void warning(){ //print warning message according to rule 5
    map<string, int>::iterator map_it = symbol_table.begin();
    list<int>::iterator list_it = actual_mdef_num.begin(); //store the actual def in each module
    list<int>::iterator length_it = module_length.begin(); //store the length of each module
    int module_no = 1; //store the module order
    int total_addr = 0;
    
    for (int i = 0; i < module_num; i++) {
        for (int j = 0; j < *list_it; j++) {
            if ((map_it->second - total_addr) > *length_it) {
                cout<< "Warning: Module " << module_no << ": " << map_it->first << " to big " << map_it->second << " (max=" << *length_it-1 << ") assume zero relative" << endl;
                map_it->second = 0;
            }
            map_it++;
        }
        module_no++;
        total_addr = total_addr + *length_it;
        length_it++;
        list_it++;
    }
    return;
}

void print_sb(){//print the whole symbol table
    map<string, int>::const_iterator map_it = symbol_table.begin();
    warning();
    cout<< "Symbol Table" << endl;
    bool flag = false;
    
    if (multi_def.empty()) {
        while (map_it != symbol_table.end()) {
            cout<< map_it->first << "=" << map_it->second << endl;
            map_it++;
        }
    }
    else{
        while (map_it != symbol_table.end()) {
            list<string>::iterator multi_it = multi_def.begin(); // There exist multi-defined symbol
            while (multi_it != multi_def.end()) {
                if (map_it->first == *multi_it) {
                    flag = true;
                }
                multi_it++;
            }
            if (flag == true) {
                cout<< map_it->first << "=" << map_it->second;
                cout<< " Error: This variable is multiple times defined; first value used" << endl;
            }
            else{
                cout<< map_it->first << "=" << map_it->second << endl;
            }
            map_it++;
            flag = false;
        }
    }
}


void read_defls(){ //read the definition list
    stringstream ss1;
    stringstream ss2;
    int defcount; //store the number of symbol definition in a single module
    string symbol = ""; // store current symbol
    int word_address; //store current word address
    int account; // store the number of symbol def actually been put into the symbol table
    
    skip_delimiter();
    if (read_char == EOF) {
        parse_error(0, 1);
    }
    ss1 << read_int();
    ss1 >> defcount; // get the defcount in the input file
    ss1.clear();
    module_def_num.push_back(defcount);// store the defcount in the current module
    account = defcount;
    if(defcount > 16){
        parse_error(4, 0);
    }
    if (defcount == 0) {
        actual_mdef_num.push_back(account);
        return;
    }
    if (read_char == EOF) {
        parse_error(1, 1);
    }
    for (int i = 0; i < defcount; i++) {
        skip_delimiter();
        if (read_char == EOF) {
            parse_error(1, 0);
        }
        symbol = read_symbol();
        if (symbol.length() > 16) { // check if the symbol length exceeds 16 characters
            parse_error(3, 0);
        }
        if (read_char == EOF) {
            parse_error(0, 0);
        }
        skip_delimiter();
        if (read_char == EOF) {
            parse_error(0, 0);
        }
        ss2 << read_int();
        ss2 >> word_address;
        ss2.clear();
        if (symbol_table.count(symbol)) { // if the symbol is multidefined, push it to multi_def list for warning message
            multi_def.push_back(symbol);
            account--;
        }
        else{
            symbol_table.insert(pair<string, int>(symbol, word_address + ab_addr));
        }
    }
    actual_mdef_num.push_back(account);
    return;
}


void skip_usels(){ //scan the uselist
    stringstream ss;
    int usecount;
    string temp = "";
    if (read_char == EOF) {
        parse_error(0, 0);
    }
    skip_delimiter();
    if (read_char == EOF) {
        parse_error(1, 0);
    }
    ss << read_int();
    ss >> usecount;
    ss.clear();
    if(usecount > 16){ //check if usecount exceeds maximum
        parse_error(5, 0);
    }
    if (read_char == EOF) {
        parse_error(1, 0);
    }
    if (usecount == 0) {
        return;
    }
    for (int i = 0; i < usecount; i++) {
        skip_delimiter();
        if (read_char == EOF) {
            parse_error(1, 1);
        }
        temp = read_symbol();
        if (temp.length() > 16) {
            parse_error(3, 1);
        }
    }
    return;
}


void read_instrls(){ //read the instruction list
    stringstream ss;
    int codecount;
    if (read_char == EOF) {
        parse_error(0, 0);
    }
    skip_delimiter();
    if (read_char == EOF) {
        parse_error(0, 0);
    }
    ss << read_int();
    ss >> codecount;
    ss.clear(); //read codecount
    if (codecount >= MAX_ADDR) {
        parse_error(6, 0);
    }
    ab_addr = ab_addr + codecount; //add module length
    module_length.push_back(codecount); //store the current module length list
    if (read_char == EOF) {
        parse_error(1, 0);
    }
    skip_delimiter();
    for (int i = 0; i < codecount; i++) {
        read_instr();
    }
    return;
}

void pass_one(){ //the first pass
    read_defls();
    skip_usels();
    read_instrls();
    return;
}

void skip_defls(){ //skip definition list
    stringstream ss1;
    stringstream ss2;
    int defcount; //store the number of symbol definition in a single module
    string symbol = ""; // store current symbol
    int word_address; //store current word address
    
    skip_delimiter();
    ss1 << read_int();
    ss1 >> defcount;
    ss1.clear(); // get the defcount in the input file
    
    for (int i = 0; i < defcount; i++) {
        skip_delimiter();
        symbol = read_symbol();
        skip_delimiter();
        ss2 << read_int();
        ss2 >> word_address;
        ss2.clear();
    }
    return;
}

string map_order(int order){
    stringstream ss;
    string addr_order = "";
    if ((order + ab_addr) <10) {
        ss << order + ab_addr;
        ss >> addr_order;
        ss.clear();
        addr_order = "00" + addr_order;
    }
    if ((order + ab_addr) >= 10 && (order + ab_addr) < 100) {
        ss << order + ab_addr;
        ss >> addr_order;
        ss.clear();
        addr_order = "0" + addr_order;
    }
    if ((order + ab_addr) >= 100) {
        ss << order + ab_addr;
        ss >> addr_order;
        ss.clear();
    }
    return addr_order;
}

void proc_i(string addr_instr, int order){
    string addr_order = map_order(order);
    if (addr_instr.length() > 4) {
        addr_instr = "9999";
        cout<< addr_order << ": " << addr_instr << " ";
        cal_error(3);
    }
    else if (addr_instr.length() <= 4) {
        int crack = 4 - (int)addr_instr.length();
        for (int i = 0; i < crack; i++) {
            addr_instr = "0" + addr_instr;
        }
        cout<< addr_order << ": " << addr_instr << endl;
    }
    return;
}

void proc_a(string addr_instr, int order){
    stringstream ss;
    string addr_order = map_order(order);
    if (addr_instr.length() > 4) {
        addr_instr = "9999";
        cout<< addr_order << ": " << addr_instr << " ";
        cal_error(4);
    }
    else if (addr_instr.length() <= 4) {
        int temp;
        ss << addr_instr;
        ss >> temp;
        ss.clear();
        if (temp % 1000 >= MAX_ADDR) {
            temp = (int)temp/1000;
            ss << temp;
            ss >> addr_instr;
            addr_instr =  addr_instr + "000";
            cout<< addr_order << ": " << addr_instr << " ";
            cal_error(0);
        }
        else{
            int crack = 4 - (int)addr_instr.length();
            for (int i = 0; i < crack; i++) {
                addr_instr = "0" + addr_instr;
            }
            cout<< addr_order << ": " << addr_instr << endl;
        }
    }
    return;
}

void proc_r(string addr_instr, int order){
    stringstream ss;
    stringstream sso;
    list<int>::const_iterator length_it = module_length.begin();
    for (int i = 0; i < module_num - 1; i++) {
        length_it++;
    }
    string addr_order = map_order(order);
    if (addr_instr.length() > 4) {
        addr_instr = "9999";
        cout<< addr_order << ": " << addr_instr << " ";
        cal_error(4);
    }
    else if (addr_instr.length() <= 4) {
        int temp;
        ss << addr_instr;
        ss >> temp;
        ss.clear();
        if ((temp % 1000) >= (*length_it)) {
            sso << (int)temp/1000;
            sso >> addr_instr;
            sso.clear();
            addr_instr = addr_instr + "000";
            ss << addr_instr;
            ss >> temp;
            ss.clear();
            temp = temp + ab_addr;
            sso << temp;
            sso >> addr_instr;
            sso.clear();
            cout<< addr_order << ": " << addr_instr << " ";
            cal_error(1);
        }
        else{
            ss << addr_instr;
            ss >> temp;
            ss.clear();
            temp = temp + ab_addr;
            sso << temp;
            sso >> addr_instr;
            sso.clear();
            cout<< addr_order << ": " << addr_instr << endl;
        }
    }
    return;
}

void proc_e(string addr_instr, int order, vector<string> usels, vector<string> & used_sm){
    stringstream ss;
    stringstream sso;
    string addr_order = map_order(order);
    map<string, int>::const_iterator map_it = symbol_table.begin();
    if (addr_instr.length() > 4) {
        addr_instr = "9999";
        cout<< addr_order << ": " << addr_instr << " ";
        cal_error(4);
    }
    else {
        int temp;
        ss << addr_instr;
        ss >> temp;
        ss.clear();
        if ((temp % 10) >= usels.size()) {
            int crack = 4 - (int)addr_instr.length();
            for (int i = 0; i < crack; i++) {
                addr_instr = "0" + addr_instr;
            }
            cout<< addr_order << ": " << addr_instr << " ";
            cal_error(2);
        }
        else{
            string temp_symbol = usels[temp % 100];
            used_symbol.push_back(temp_symbol); //put the symbol in the used list for warning
            used_sm.push_back(temp_symbol);
            map_it = symbol_table.find(temp_symbol);
            if (map_it != symbol_table.end()) {
                temp = temp - (temp % 100) + map_it->second;
                sso << (int)temp;
                sso >> addr_instr;
                sso.clear();
                int crack = 4 - (int)addr_instr.length();
                for (int i = 0; i < crack; i++) {
                    addr_instr = "0" + addr_instr;
                }
                cout<< addr_order << ": " << addr_instr << endl;
            }
            else{
                temp = temp - (temp % 100);
                sso << (int)temp;
                sso >> addr_instr;
                sso.clear();
                int crack = 4 - (int)addr_instr.length();
                for (int i = 0; i < crack; i++) {
                    addr_instr = "0" + addr_instr;
                }
                cout<< addr_order << ": " << addr_instr << " ";
                cout<< "Error: "<< temp_symbol << " is not defined; zero used" << endl;
            }
        }
        
    }
    return;
}

void cal_usels_instrls(){ //store uselist and calculate the instrls
    stringstream ss;
    int usecount;
    int codecount;
    string temp = ""; //store current external symbol in the list
    string addr_instr = "";
    char addr_type;
    vector<string> usels;
    vector<string> used_sm;//store used external symbol in each symbol
    
    skip_delimiter();//read external symbols in the uselist
    ss << read_int();
    ss >> usecount;
    ss.clear();
    
    for (int i = 0; i < usecount; i++) {
        skip_delimiter();
        temp = read_symbol();
        usels.push_back(temp);
    }
    
    skip_delimiter();
    ss << read_int();
    ss >> codecount;
    ss.clear();//read codecount
    skip_delimiter();
    for (int i = 0; i < codecount; i++) {
        addr_type = read_type();
        skip_delimiter();
        addr_instr = read_addr();
        skip_delimiter();
        switch (addr_type) {
            case 'I':
                proc_i(addr_instr, i);
                break;
            case 'A':
                proc_a(addr_instr, i);
                break;
            case 'R':
                proc_r(addr_instr, i);
                break;
            default:
                proc_e(addr_instr, i, usels, used_sm);
                break;
        }// calculate the address according to the addr_type & add_instr
    }
    for (int j = 0; j < usels.size(); j++) {
        vector<string>::iterator sm_it = used_sm.begin();//print warning message after the current module
        while (sm_it != used_sm.end()) {
            if (*sm_it == usels[j]) {
                break;
            }
            sm_it++;
        }
        if (sm_it == used_sm.end()) {
            cout<< "Warning: Module " << module_num << ": " << usels[j] << " appeared in the uselist but was not actually used" << endl;
        }
    }
    used_sm.erase(used_sm.begin(), used_sm.end());//reset used_sm to check the next module
    ab_addr = ab_addr + codecount; //add module length
    return;
}

void n_warning(){
    map<string, int>::const_iterator map_it = symbol_table.begin();
    list<int>::const_iterator list_it = actual_mdef_num.begin();
    int module_no = 1;
    
    while (map_it != symbol_table.end()) {
        for (int i = 0; i < *list_it; i++) {
            list<string>::const_iterator ls_it = used_symbol.begin();
            while (ls_it != used_symbol.end()) {
                if (*ls_it == map_it->first) {
                    break;
                }
                ls_it++;
            }
            if (ls_it == used_symbol.end()) {
                cout<< "Warning: Module " << module_no << ": " << map_it->first << " was defined but never used"<< endl;
            }
            map_it++;
        }
        list_it++;
        module_no++;
    }
}


void pass_two(){
    skip_defls();
    cal_usels_instrls();
    return;
}


int main(int argc, const char * argv[]) {
    pFile=fopen (argv[1],"r");
    if (pFile==NULL) {
        cout<< "Error opening file";
        return 0;
    }
    read_char = fgetc(pFile);
    while (read_char != EOF) { //pass one
        module_num++;
        pass_one();
        skip_delimiter();
    }
    fclose (pFile);
    print_sb();
    
    cline = 1; //reset all the line and line offset
    cl_offset = 1;
    pline = 1;
    pl_offset = 1;
    ab_addr = 0;
    module_num = 0;
    cout<< endl;
    
    pFile=fopen (argv[1],"r");
    read_char = fgetc(pFile);
    cout << "Memory Map" << endl;
    while (read_char != EOF) { //pass two
        module_num++;
        pass_two();
        skip_delimiter();
    }
    cout<< endl;
    n_warning();
    fclose (pFile);
    return 0;
}
