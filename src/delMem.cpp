#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <semaphore.h>

using namespace std;

int
main(void) {
  shm_unlink("evaluator");
  string nombre;
  string nombreSeg = "evaluator";
 for (int i = 0; i < 5 ; i++ ){
        nombre =  nombreSeg + "-vacios" + to_string(i);
        sem_unlink(nombre.c_str());
        nombre =  nombreSeg + "-llenos" + to_string(i);
        sem_unlink(nombre.c_str());
        nombre =  nombreSeg + "-mutex" + to_string(i);
        sem_unlink(nombre.c_str());
  }

  char letra = 'B';
  for (int i = 0; i < 3; i++) {
    if (i == 1) letra ='S';
    if (i == 2) letra ='D';
    nombre =  nombreSeg + "vacios"+ letra;
    sem_unlink(nombre.c_str());
    nombre =  nombreSeg + "llenos"+ letra;
    sem_unlink(nombre.c_str());
    nombre =  nombreSeg + "mutex" + letra;
    sem_unlink(nombre.c_str());
  }
  
  nombre = nombreSeg + "vaciosS";
  sem_unlink(nombre.c_str());
  nombre = nombreSeg + "llenosS";
  sem_unlink(nombre.c_str());
  nombre = nombreSeg + "mutexS";
  sem_unlink(nombre.c_str());

  return EXIT_SUCCESS;
}
