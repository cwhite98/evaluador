#pragma once
#include <string>
#include <vector>
#include "init.h"

using namespace std;

class Ctrl {
  public:
    Ctrl();
   ~Ctrl();
   void control(string);
   void processing(struct Examen* pI,string segName,int tamColasI);
    void waiting(Examen*[],int,int,string);
    void reported(Examen*,struct Header *pH,string);
    void reactive(Header *pHeader);
};
