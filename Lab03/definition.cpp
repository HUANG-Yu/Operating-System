#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

struct flaggroup {
    bool flago;
    bool flagp;
    bool flagf;
    bool flags;
};

struct freelist {
    int vp;
    unsigned int no;
};

struct pte {
    unsigned int present: 1;
    unsigned int modified: 1;
    unsigned int referenced: 1;
    unsigned int pageout: 1;
    unsigned int reserved :  22;
    unsigned int index: 6;
};

struct counter {
    unsigned long int unmap: 64;
    unsigned long int map: 64;
    unsigned long int pagein: 64;
    unsigned long int pageout: 64;
    unsigned long int zero: 64;
    long int instr: 64;
};