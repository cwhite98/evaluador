#pragma once
#include <string>
#include "elementos.h"
using namespace std;

class Reg {
    public:
    void leerArchivo(string,string);
    void interactivo(string);
    void* openMem(string);
    Reg();
    ~Reg();

    private:
    int id;
};

extern bool mode;