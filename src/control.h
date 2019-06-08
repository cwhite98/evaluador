#pragma once
#include <string>


using namespace std;

class Ctrl {
  public:
    Ctrl();
   ~Ctrl();
    void control(string);
    void processing(vector<Examen>,vector<Examen>,vector<Examen>);
    void waiting(Examen*[],struct Header *pH);
    void reported(Examen*,struct Header *pH);
    void reactive(Header *pHeader);
};
