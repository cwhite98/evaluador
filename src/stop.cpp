#include "stop.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include "elementos.h"

Stop::Stop(){};
Stop::~Stop(){};

using namespace std;

void Stop::borrar(std::string segName){
    int fd = shm_open(segName.c_str(),O_RDWR,0660);
    struct Header *pHeader = (struct Header*) mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    int numColas = pHeader->numColasE;
    close(fd);
    shm_unlink(segName.c_str());
    string nombre;
    for (int i = 0; i < numColas ; i++ ){
        nombre =  segName + "-vacios" + to_string(i);
        sem_unlink(nombre.c_str());
        nombre =  segName + "-llenos" + to_string(i);
        sem_unlink(nombre.c_str());
        nombre =  segName + "-mutex" + to_string(i);
        sem_unlink(nombre.c_str());
    }

    char letra = 'B';
    for (int i = 0; i < 3; i++) {
        if (i == 1) letra ='S';
        if (i == 2) letra ='D';
        nombre =  segName + "vacios"+ letra;
        sem_unlink(nombre.c_str());
        nombre =  segName + "llenos"+ letra;
        sem_unlink(nombre.c_str());
        nombre =  segName + "mutex" + letra;
        sem_unlink(nombre.c_str());
    }
    
    nombre = segName + "vaciosS";
    sem_unlink(nombre.c_str());
    nombre = segName + "llenosS";
    sem_unlink(nombre.c_str());
    nombre = segName + "mutexS";
    sem_unlink(nombre.c_str());

}