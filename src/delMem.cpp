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

  return EXIT_SUCCESS;
}
