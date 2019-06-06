#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <pthread.h>
#include "elementos.h"
#include "init.h"

using namespace std;

Init::Init() {}
Init::~Init() {}
queue<Examen> blood, skin, detritus;
sem_t *mutexB, *vaciosB, *llenosB;
sem_t *mutexS, *vaciosS, *llenosS;
sem_t *mutexD, *vaciosD, *llenosD;

void Init::init(int i, int ie, int oe , string n, int b, int d, int s, int q) {
    int tamColasE = ie;
    int tamColasS = oe;
    int numColasE = i;
    int rBlood = b;
    int rSkin = s;
    int rDet = d;     
    int tamColasI = q;
    segName = n;

    int fd = shm_open(segName.c_str(), O_RDWR | O_CREAT | O_EXCL, 0660); 

    // ftruncate amplia el segmento al tamaño que necesitamos
    ftruncate(fd, sizeof(struct Header) + (sizeof(struct Examen)*numColasE * tamColasE )+ (sizeof(struct Examen) * tamColasS));
    void *dir = mmap(NULL,sizeof(struct Header) + (sizeof(struct Examen)*numColasE * tamColasE )+ (sizeof(struct Examen) * tamColasS), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);

    struct  Header *pHeader = (struct Header*) dir;
    pHeader->tamColasE = tamColasE;
    pHeader->numColasE = numColasE;
    pHeader->rBlood = rBlood;
    pHeader->rSkin = rSkin;
    pHeader->rDet = rDet;
    pHeader->tamColasS = tamColasS;

    close(fd);
    evaluador(pHeader,segName);
    munmap(dir,sizeof(struct Header) + (sizeof(struct Examen)*numColasE * tamColasE )+ (sizeof(struct Examen) * tamColasS));
}

void Init::evaluador( Header *pHeader, string segName) {
    cout << "Evaluador " << endl;
    string nombre;
    int numColasE = pHeader->numColasE;
    pthread_t entrada[numColasE];

    sem_t *mutex[numColasE];
    sem_t *vacios[numColasE];
    sem_t *llenos[numColasE];

    Examen **colaExamenes = new Examen*[numColasE];

    void *dir;
    pthread_info pi;
    for (int i = 0; i < numColasE ; i++) {
        nombre =  segName + "-mutex" + to_string(i);
        mutex[i] = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 1);
        nombre =   segName + "-vacios" + to_string(i);
        vacios[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, pHeader->tamColasE-1);
        nombre =  segName + "-llenos" + to_string(i);
        llenos[i] = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);
        if (i==0) {
            dir = ((char*)pHeader) + sizeof(struct Header);
        } else {
            dir = ((char*)dir) + sizeof(struct Examen) *  pHeader->tamColasE;
        }
        pi.pCola = (struct Examen*) dir;
        pi.mutex = mutex[i];
        pi.llenos = llenos[i];
        pi.vacios = vacios[i];
        pi.tamano = pHeader->tamColasE;
        
        pthread_create(&entrada[i], NULL, leerEntrada,&pi) ;
        sleep(1);
    }

    for (int i = 0; i < 1; i++) {
        pthread_join(entrada[i], NULL);
    }


        nombre = segName + "mutexB";
        mutexB = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);
        nombre = segName + "vaciosB";
        vaciosB = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);
        nombre = segName + "llenosB";
        llenosB = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);
        nombre = segName + "mutexS";
        mutexS = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);
        nombre = segName + "vaciosS";
        vaciosS = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);
        nombre = segName + "llenosS";
        llenosS = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);

        nombre = segName + "mutexD";
        mutexD = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);
        nombre = segName + "vaciosD";
        vaciosD = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);
        nombre = segName + "llenosD";
        llenosD = sem_open(nombre.c_str(),O_CREAT | O_EXCL, 0660, 0);

}

void* Init::leerEntrada(void *arg) {
    cout << "Leer entrada " << endl;
    pthread_info *args = (pthread_info*) arg;
    struct Examen *ex = args->pCola; 
    Examen examen;
    for(;;) {
        cout << "Hay?" << endl;
        sem_wait(args->llenos);
        cout << " Si Hay?" << endl;
        sem_wait(args->mutex);
        examen.tipo = ex->tipo;
        examen.id = ex->id;
        examen.numeroMuestras = ex->numeroMuestras;
        examen.numCola = ex->numCola;
        cout << ex->tipo << endl;
        if( examen.tipo == 'B') {
            sem_wait(vaciosB);
            sem_wait(mutexB);
            blood.push(examen);
            sem_wait(mutexB);
            sem_wait(llenosB);
        } else if (examen.tipo == 'S'){
            sem_wait(vaciosS);
            sem_wait(mutexS);
            skin.push(examen);
            sem_wait(mutexS);
            sem_wait(llenosS);
        } else if (examen.tipo == 'D'){
            sem_wait(vaciosD);
            sem_wait(mutexD);
            detritus.push(examen);
            sem_wait(mutexD);
            sem_wait(llenosD);
        }

        sem_post(args->mutex);
        sem_post(args->vacios);
        sleep(2);

    }
}