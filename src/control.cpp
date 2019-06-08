#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include "elementos.h"
#include "control.h"
#include "init.h"

using namespace std;

Ctrl::Ctrl() {};
Ctrl::~Ctrl() {};

void Ctrl::control(string segName) {
  int fd = shm_open(segName.c_str(),O_RDWR,0660);
  void* dir0;
  cout << "Haciendo mmap " << endl;
  dir0 = mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
  struct Header *pHeader = (struct Header*) dir0;
  int numColasE = pHeader->numColasE;
  int tamColasE = pHeader->tamColasE;
  int tamColasS = pHeader->tamColasS;
  munmap(dir0,sizeof(struct Header));
  dir0 = mmap(NULL,sizeof(struct Header)+ sizeof(struct Examen)*numColasE*tamColasE + sizeof(struct Examen)*tamColasS, PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
  Examen *pCola[numColasE+1];
  void* dir[numColasE];
  dir[0] = ((char*)dir0) + sizeof(struct Header);
  pCola[0] = (struct Examen*) dir[0];
  for(int i = 1;i < numColasE+1; i++) {
    dir[i] = ((char*)dir[i-1]) + sizeof(struct Examen) *  pHeader->tamColasE;
    pCola[i] = (struct Examen*) dir[i];
  }
  Init init();
  Vector<Examen> blood = init.getBloodV();
  Vector<Examen> skin = init.getSkinV();
  Vector<Examen> detritus = init.getDetrotusV();
  string linea;
  cout << "antes while getline" <<endl;
  while(getline(cin,linea)){
    if(linea.find("list") != string::npos){
      string list = linea.substr(linea.find("list"));
      cout << "Detectó: "<< list<<endl ;
      string modo = linea.substr(linea.find("list"),linea.size());
      cout << "Detecto modo " << modo << endl;
      modo.erase(remove_if(str.begin(), str.end(), isspace), str.end());
      if(modo == "processing"){
        processing(blood,skin,detritus);
      }else if(modo == "waiting"){
        waiting(pCola,pHeader);
      }else if(modo == "reported"){
        reported(pCola[tamColasE],pHeader);
      }else if(modo == "reactive"){
        reactive(pHeader);
      }else if(modo == "all"){
        cout << "------------------ALL------------------------" <<endl;
        cout << endl;
        processing(blood,skin,detritus);
        waiting(pCola,pHeader);
        reported(pCola[tamColasE],pHeader);
        reactive(pHeader);
      }else if(modo.empty()){
        continue;
      }else{
        //wrong use
      }
    }else if(linea.find("update") != string::npos){
      string update = linea.substr(linea.find("update"));
      cout << "Detecto:  "<< list<<endl ;
      string proximo = linea.substr(linea.find("update"),linea.size());
      string tipo;
      int cantidad;
      if(proximo.find("B") != string::npos){
        tipo = proximo.substr(proximo.find("B"));
        cantidad = atoi(proximo.substr(proximo.find("B"),proximo.size()).c_str());
        //resolver sobre paso
        pHeader->rBlood = pHeader->rBlood  + cantidad;
      } else if(proximo.find("D") != string::npos){
        tipo = proximo.substr(proximo.find("D"));
        cantidad = atoi(proximo.substr(proximo.find("D"),proximo.size()).c_str());
        //resolver sobre paso
        pHeader->rDet = pHeader->rDet  + cantidad;
      }else if(proximo.find("S") != string::npos){
        tipo = proximo.substr(proximo.find("S"));
        cantidad = atoi(proximo.substr(proximo.find("S"),proximo.size()).c_str());
        //resolver sobre paso
        pHeader->rSkin = pHeader->rSkin  + cantidad;
      }else{
        //mal uso
      }
      cout << "Es tipo: " << tipo << endl;
      cout << "Cantidad: "<< cantidad <<endl;
    }else{
      //mal uso
    }
  }
}

void Ctrl::processing(vector<Examen> blood,vector<Examen> skin,vector<Examen> detritus){
  cout << "------------------PROCESSING------------------------" <<endl;
  //mutex intermedio blood
  for (vector<Examen>::const_iterator i = blood.begin(); i != blood.end(); ++i){
    cout <<" | "<<*i->id << " | " << *i->numCola << " | " 
    << *i->tipo << " | " << *i->numeroMuestras << " | " 
    << *i->tiempo << " |"<< endl ; 
  }
  //soltar mutex blood

  //mutex intermedio skin
  for (vector<Examen>::const_iterator i = skin.begin(); i != skin.end(); ++i){
    cout <<" | "<<*i->id << " | " << *i->numCola << " | " 
    << *i->tipo << " | " << *i->numeroMuestras << " | " 
    << *i->tiempo << " |"<< endl ; 
  }
   //soltar intermedio skin

  //mutex intermedio detritus
  for (vector<Examen>::const_iterator i = detritus.begin(); i != detritus.end(); ++i){
    cout <<" | "<<*i->id << " | " << *i->numCola << " | " 
    << *i->tipo << " | " << *i->numeroMuestras << " | " 
    << *i->tiempo << " |"<< endl ; 
  }
  //soltar intermedio detritus
}

void Ctrl::waiting(Examen *pCola[],Header *pHeader){
  cout << "------------------WAITING------------------------" <<endl;
  Examen * examenCopia;
  for(int s = 0 ; s < pHeader->numColasE; s++){
    //mutex de esa cola
    for(int v = 0; v < pHeader->tamColasE; v++){
      examenCopia = (struct Examen*)((char *)pCola[s]) + (sizeof(struct Examen)*v);
      if(examenCopia->numeroMuestras != 0){
        cout <<" | "<<examenCopia->id << " | " << examenCopia->numCola << " | " 
        << examenCopia->tipo << " | " << examenCopia->numeroMuestras << " |"<< endl ; 
      }
    }
    //soltar mutex de esa cola
  }
}

void Ctrl::reported(Examen *colaSalida,Header *pHeader){
  cout << "------------------REPORTED------------------------" <<endl;
  Examen *examenCopia;
  //agarrar el mutex de la cola salida
  for(int s = 0; s < pHeader->tamColasS;s++){
    examenCopia = (struct Examen*)((char*) colaSalida) + (sizeof(struct Examen) * s);
    if(examenCopia->numeroMuestras !=0){
      cout <<" | "<<examenCopia->id << " | " << examenCopia->numCola << " | " 
      << examenCopia->tipo << " | " << examenCopia->numeroMuestras << " | " 
      << examenCopia->resultado << " |"<< endl ; 
    }
  }
  //soltar el mutex de la cola salida
}

void Ctrl::reactive(Header *pHeader){
  cout << "------------------REPORTED------------------------" <<endl;
  cout <<" | BLOOD: "<<pHeader->rBlood << " | " << "SKIN: "<<pHeader->rSkin << " | "
  << "DETRITUS: "<<pHeader->rDet << endl; 
}
