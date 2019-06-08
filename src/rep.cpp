#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <semaphore.h>
#include <list>
#include "elementos.h"
#include "rep.h"
#include "init.h"

using namespace std;

Rep::Rep(){};
Rep::~Rep(){};

void Rep::report(string segName, char c, int i) {
  string nombre;
  int fd = shm_open(segName.c_str(), O_RDWR, 0660);
  void *dir0;
  dir0 = mmap(NULL, sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  struct Header *pHeader = (struct Header *)dir0;
  int numColasE = pHeader->numColasE;
  int tamColasE = pHeader->tamColasE;
  int tamColasS = pHeader->tamColasS;
  
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

  Init init;
  nombre = segName + "vaciosS";
  sem_t *vaciosS =  sem_open(nombre.c_str(), 0);
  sem_t *mutexS = init.getSemMutex();
  sem_t *llenosS = init.getSemLlenos();
  void *d;
  Examen *copia = pCola[numColasE];
  cout << copia->numeroMuestras << endl;
  if (c == 'i') {
    cout << "ENTRE A I" << endl;
    int pos;
    while (true) {
      sem_wait(llenosS);
      sem_wait(mutexS);
      while (copia->numeroMuestras != 0) {
        if (pos == 0) {
          copia = pCola[numColasE];
        }  
        cout << copia->id << " ";
        cout << copia->numCola << " "
             << copia->tipo << " "
             << copia->resultado << endl;
        Examen * res = copia;
        res->numeroMuestras = 0;
        pos++;
        if (pos == tamColasS) {
            pos = 0;
            copia =  pCola[numColasE];
        }else{
          d = ((char *)copia) + (sizeof(struct Examen));
          copia = (struct Examen *) d;
        }
        if (copia->numeroMuestras == 0) {
          pos = 0;
          copia =  pCola[numColasE];
        }
        cout << "rep while" << endl;
      }

      sem_post(mutexS);
      sem_post(vaciosS);
      cout << "Esperando " << endl;
      sleep(i);
      
    }
  } else {
    int cont = 0;
    int pos = 0;
    int pos2 = 0;
    list<int> revisado;
    copia = pCola[numColasE];
    while (true) {
      sem_wait(llenosS);
      sem_wait(mutexS);
      cont = 0;
      revisado.clear();
      copia = pCola[numColasE];
      while(cont < i) {
        while (copia->numeroMuestras != 0 && cont < i && pos < tamColasS) {
          if (pos == 0) {
            copia = pCola[numColasE];
          }
          cont++;
          pos++;
          if (copia->numeroMuestras == 0 || pos== tamColasS) {
            break;
          }
          d = ((char *)copia) + (sizeof(struct Examen));
          copia = (struct Examen *) d;
        }
      }
      pos = 0;
      copia = pCola[numColasE];
      
      while (revisado.size() < i) {
        if (pos2 == 0) {
          copia = pCola[numColasE];
        }   else{
          d = ((char *)copia) + (sizeof(struct Examen));
          copia = (struct Examen *) d;
        }

        Examen * res = copia;
        if(copia->numeroMuestras != 0  ){
          revisado.push_back(res->id);
          cout << copia->id << " ";
          cout << copia->numCola << " "
             << copia->tipo << " "
             << copia->resultado << endl;
          revisado.unique();
          cout << "$$$$$$$$$$ "<<revisado.size() << endl;
          res->numeroMuestras = 0;
        }
        pos2++;
        if (pos2 == tamColasS) {
            pos2 = 0;
            pos = 0;
            copia =  pCola[numColasE];
        }
        cout << "rep while" << endl;
      }
      pos = pos2;
      sem_post(mutexS);
      sem_post(vaciosS);
    }
  }
}
