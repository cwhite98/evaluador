#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include "elementos.h"
#include "rep.h"

using namespace std;

Rep::Rep() {};
Rep::~Rep() {};

void Rep::report(string segName, char c, int i) {
  int fd = shm_open(segName.c_str(),O_RDWR,0660);
  void* dir0;
  dir0 = mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
  struct Header *pHeader = (struct Header*) dir0;
  int numColasE = pHeader->numColasE;
  int tamColasE = pHeader->tamColasE;
  int tamColasS = pHeader->tamColasS;
  munmap(dir0,sizeof(struct Header));
    dir0 = mmap(NULL,sizeof(struct Header)+ sizeof(struct Examen)*numColasE*tamColasE + sizeof(struct Examen)*tamColasS, PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    Examen *pCola[numColasE];
    void* dir[numColasE];
    dir[0] = ((char*)dir0) + sizeof(struct Header);
    pCola[0] = (struct Examen*) dir[0];
    for(int i = 1;i < numColasE; i++) {
        dir[i] = ((char*)dir[i-1]) + sizeof(struct Examen) *  pHeader->tamColasE;
        pCola[i] = (struct Examen*) dir[i];
    }
    Examen *colaSalida = (struct Examen*)((char *) pCola[numColasE-1]) + (sizeof(struct Examen) * pHeader->tamColasE);
    cout << colaSalida->id << endl;
    cout << colaSalida->numeroMuestras<<endl;

 
  
  cout << "EmpezaraWhite"<< endl;
  cout << colaSalida->id<<endl;
  cout << colaSalida->numeroMuestras <<endl; 
  if(c == 'i') {
    while(true) {
      while(colaSalida->numeroMuestras != 0) {
	colaSalida = (struct Examen *)((char *)colaSalida) + (sizeof(struct Examen));
	cout << colaSalida->id << " ";
	cout << colaSalida->numCola << " "
	     << colaSalida->tipo << " "
	     << colaSalida->resultado << endl;
      }
      sleep(i);
    }
  }
}
