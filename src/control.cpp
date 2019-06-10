#include <iostream>
#include <algorithm>
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
#include <pthread.h>
using namespace std;

Ctrl::Ctrl() {};
Ctrl::~Ctrl() {};

void Ctrl::control(string segName) {
  int fd = shm_open(segName.c_str(), O_RDWR, 0660);
  void *dir0;
  dir0 = mmap(NULL, sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  struct Header *pHeader = (struct Header *)dir0;
  int numColasE = pHeader->numColasE;
  int tamColasE = pHeader->tamColasE;
  int tamColasS = pHeader->tamColasS;
  int tamColasI = pHeader->tamColasI;
  munmap(dir0, sizeof(struct Header));

  dir0 = mmap(NULL, sizeof(struct Header) + sizeof(struct Examen) * numColasE * tamColasE + sizeof(struct Examen) * tamColasS, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);


  Examen *pCola[numColasE + 1];
  void *dir[numColasE];
  dir[0] = ((char *)dir0) + sizeof(struct Header);
  pCola[0] = (struct Examen *)dir[0];
  for (int i = 1; i < numColasE + 1; i++) {
    dir[i] = ((char *)dir[i - 1]) + sizeof(struct Examen) * pHeader->tamColasE;
    pCola[i] = (struct Examen *)dir[i];
  }
  pHeader = (struct Header*)dir0;
  close(fd);
  string nameColasI = "colasInt";
  int colasInt = shm_open(nameColasI.c_str(), O_RDWR, 0660);
  void *dirI = mmap(NULL, (sizeof(struct Examen))*tamColasI*3, PROT_READ | PROT_WRITE, MAP_SHARED, colasInt, 0);
  close(colasInt);
  Examen *pI[3];
  void *dirInt[3];
  dirInt[0] = ((char *)dirI); // Direccion primer examen
  pI[0] = (struct Examen *)((char *)dirInt[0]);    //primera direccion inicio cola
  for (int i = 1; i < 3; i++) {
    dirInt[i] = ((char *)dirInt[i - 1]) + (sizeof(struct Examen) * pHeader->tamColasI); // Siguiente direccion
    pI[i] = (struct Examen *)dirInt[i];   // Siguiente cola
  }

  string linea;
  cout << "> ";
  while(getline(cin,linea)){
    if(linea.find("list") != string::npos){
      string list = linea.substr(linea.find("list"),linea.find("list") + 4);
      string modo = linea.substr(linea.find("list")+4,linea.size());
      string::iterator end_pos = remove(modo.begin(),modo.end(),' ');
      modo.erase(end_pos,modo.end());
      if(modo == "processing") {
        processing(pI[0],segName,tamColasI);
      }else if(modo == "waiting") {
        waiting(pCola,pHeader->numColasE,pHeader->tamColasE,segName);
      }else if(modo == "reported"){
        reported(pCola[numColasE],pHeader,segName);;
      }else if(modo == "reactive"){
        reactive(pHeader);
      }else if(modo == "all"){
        cout << endl;
        processing(pI[0],segName,tamColasI);
        waiting(pCola,pHeader->numColasE,pHeader->tamColasE,segName);
        reported(pCola[tamColasE],pHeader,segName);
        reactive(pHeader);
      }else if(modo.empty()){
        continue;
      }else{
        //wrong use
      }
    }else if(linea.find("update") != string::npos){
      string update = linea.substr(linea.find("update"),linea.find("update")+6);
      string proximo = linea.substr(linea.find("update")+6,linea.size());
      string tipo;
      int cantidad = 0;
      if(proximo.find("B") != string::npos){
        tipo = proximo.substr(proximo.find("B"), proximo.find("B")+1);
        cantidad = atoi(proximo.substr(proximo.find("B")+1,proximo.size()).c_str());
        int max = pHeader->maxBlood;
        int aux = 0;
        cout <<" | Blood Anterior: " << pHeader->rBlood 
        << " | Blood Maximo: " << pHeader->maxBlood;
        if(pHeader->rBlood  + cantidad <= max) {
          pHeader->rBlood = pHeader->rBlood + cantidad;
        } else {
          aux = cantidad - pHeader->rBlood;
          pHeader->rBlood = (max - aux) + aux;
        }
        cout << " | Blood Actualizado : " <<  pHeader->rBlood << " |" <<endl; 
      }else if(proximo.find("D") != string::npos){
        tipo = proximo.substr(proximo.find("D"), proximo.find("D")+1);
        cantidad = atoi(proximo.substr(proximo.find("D")+1,proximo.size()).c_str());
        int max = pHeader->maxDet;
        int aux = 0;
        cout <<" | Detritus Anterior: " << pHeader->rDet 
        << " | Detritus Maximo: " << pHeader->maxDet;
        if(pHeader->rDet  + cantidad <= max) {
          pHeader->rDet = pHeader->rDet + cantidad;
        } else {
          aux = cantidad - pHeader->rDet;
          pHeader->rDet = (max - aux) + aux;
        }
        cout << " | Detritus Actualizado : " <<  pHeader->rDet << " |" <<endl; 
      }else if(proximo.find("S") != string::npos){
        tipo = proximo.substr(proximo.find("S"), proximo.find("S")+1);
        cantidad = atoi(proximo.substr(proximo.find("S")+1,proximo.size()).c_str());
        int max = pHeader->maxSkin;
        int aux = 0;
        cout <<" | Skin Anterior: " << pHeader->rSkin 
        << " | Skin Maximo: " << pHeader->maxSkin;
        if(pHeader->rSkin  + cantidad <= max) {
          pHeader->rSkin = pHeader->rSkin + cantidad;
        } else {
          aux = cantidad - pHeader->rSkin;
          pHeader->rSkin = (max - aux) + aux;;
        }
        cout << " | Skin Actualizado : " <<  pHeader->rDet << " |" <<endl; 
      }else{
        //mal uso
      }
    }else{
      //mal uso
    }
    cout << "> ";
  }
}
void Ctrl::processing(Examen* pI,string segName,int tamColasI){
    cout << "------------------PROCESSING------------------------" <<endl;
    char letra = 'B';
    string nombre;
    
    sem_t *mutexI[3];
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            letra = 'S';
        } else if (i == 2) {
            letra = 'D';
        }
        nombre = segName + "mutexI" + letra;
        mutexI[i] = sem_open(nombre.c_str(),0);
        nombre = segName + "vaciosI" + letra;
    }
    sem_wait(mutexI[0]);
    struct Examen* copia = pI;
    void* dir = ((char*)pI);
    copia = (struct Examen*)dir;
    for (int i = 0; i < tamColasI; ++i)
    {
    	if(copia->numeroMuestras != 0){
			cout <<" | "<<copia->id << " | " << copia->numCola << " | " 
      		<< copia->tipo << " | " << copia->numeroMuestras << " | " 
      		<< copia->tiempo << " |"<< endl ;
    	}
    	if(i != tamColasI-1){
    		dir = ((char *)copia) + sizeof(struct Examen);
        	copia = (struct Examen *)dir;
    	}
    }
    sem_post(mutexI[0]);

    sem_wait(mutexI[1]);
    dir = ((char *)copia) + sizeof(struct Examen);
    copia = (struct Examen *)dir;
    for (int i = 0; i < tamColasI; ++i)
    {
    	if(copia->numeroMuestras != 0){
			cout <<" | "<<copia->id << " | " << copia->numCola << " | " 
      		<< copia->tipo << " | " << copia->numeroMuestras << " | " 
      		<< copia->tiempo << " |"<< endl ;
    	}
    	if(i != tamColasI-1){
    		dir = ((char *)copia) + sizeof(struct Examen);
        	copia = (struct Examen *)dir;
    	}
    }
    sem_post(mutexI[1]);

    sem_wait(mutexI[2]);
    dir = ((char *)copia) + sizeof(struct Examen);
    copia = (struct Examen *)dir;
    for (int i = 0; i < tamColasI; ++i)
    {
    	if(copia->numeroMuestras != 0){
			cout <<" | "<<copia->id << " | " << copia->numCola << " | " 
      		<< copia->tipo << " | " << copia->numeroMuestras << " | " 
      		<< copia->tiempo << " |"<< endl ;
    	}
    	if(i != tamColasI-1){
    		dir = ((char *)copia) + sizeof(struct Examen);
        	copia = (struct Examen *)dir;
    	}
    }
    sem_post(mutexI[2]);
}

void Ctrl::waiting(Examen *pCola[],int numColasE,int tamColasE,string segName){
  cout << "------------------WAITING------------------------" <<endl;
  Examen * examenCopia;
  sem_t* mutex[numColasE];
  string nombre;
  for(int s = 0 ; s < numColasE; s++){
    nombre = segName + "-mutex" + to_string(s);
    mutex[s] = sem_open(nombre.c_str(), 0);
    sem_wait(mutex[s]);
    for(int v = 0; v < tamColasE; v++){
      examenCopia = (struct Examen*)((char *)pCola[s]) + (sizeof(struct Examen)*v);
      if(examenCopia->numeroMuestras != 0){
        cout <<" | "<<examenCopia->id << " | " << examenCopia->numCola << " | " 
        << examenCopia->tipo << " | " << examenCopia->numeroMuestras << " |"<< endl ; 
      }
    }
    //sem_post(mutex[s]);
  }
}

void Ctrl::reported(Examen *colaSalida,Header *pHeader,string segName){
  cout << "------------------REPORTED------------------------" <<endl;

  Examen *examenCopia;
  sem_t *mutexS;
  string nombre = segName + "mutexS";
  mutexS = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 1);

  sem_wait(mutexS);

  for(int s = 0; s < pHeader->tamColasS;s++){
    examenCopia = (struct Examen*)((char *)colaSalida) + (sizeof(struct Examen)*s);
    if(examenCopia->numeroMuestras != 0 ){
      cout <<" | "<<examenCopia->id << " | " << examenCopia->numCola << " | " 
      << examenCopia->tipo << " | " << examenCopia->numeroMuestras << " | " 
      << examenCopia->resultado << " |"<< endl ; 
    }
  }
  sem_post(mutexS);
}

void Ctrl::reactive(Header *pHeader){
  cout << "------------------REACTIVE------------------------" <<endl;
  cout <<" | BLOOD: "<<pHeader->rBlood << " | " << "SKIN: "<<pHeader->rSkin 
  << " | " << "DETRITUS: "<<pHeader->rDet << endl; 
}
