#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include "reg.h"
#include "elementos.h"
#include "init.h"
#include <fstream>

using namespace std;

Reg::Reg() {}
Reg::~Reg() {}
bool mode;
void Reg::leerArchivo(string archivo, string nombreSeg) { 
    mode = false;
    ifstream file;
    file.open (archivo);
    string line;

    string nombre = archivo.substr (0,archivo.find_last_of('.'));
    nombre = nombre + ".spl";
    ofstream salida;
    salida.open(nombre);

    void *dir0 = openMem(nombreSeg);
    struct Header *pHeader = (struct Header*) dir0;
    int numColasE = pHeader->numColasE;
    int tamColasE = pHeader->tamColasE;
    int tamColasS = pHeader->tamColasS;

    Examen *pCola[numColasE];
    void* dir[numColasE];
    dir[0] = ((char*)dir0) + sizeof(struct Header);
    pCola[0] = (struct Examen*) dir[0];
    for(int i = 1;i < numColasE; i++) {
        dir[i] = ((char*)dir[i-1]) + sizeof(struct Examen) *  pHeader->tamColasE;
        pCola[i] = (struct Examen*) dir[i];
    }

    string word;
    int numMuestras;
    int numCola;
    char tipo;
    sem_t *vacios[numColasE], *llenos[numColasE], *mutex[numColasE];

    for (int i = 0; i < numColasE ; i++ ){
        nombre =  nombreSeg + "-vacios" + to_string(i);
        vacios[i] = sem_open(nombre.c_str(), 0);
        nombre =  nombreSeg + "-llenos" + to_string(i);
        llenos[i] = sem_open(nombre.c_str(), 0);
        nombre =  nombreSeg + "-mutex" + to_string(i);
        mutex[i]  = sem_open(nombre.c_str(), 0);
    }

    struct Examen *copia;
    int max = 2000 + numColasE *tamColasE + tamColasS;
    srand(time(NULL));
    while (file >> word) {
        numCola = stoi(word);
        file >> word;
        tipo = word[0u];
        file >> word;
        numMuestras = stoi(word);
        if(numMuestras < 0 || numMuestras > 5) {
            continue;
        }
        if(numCola > numColasE - 1 || numCola < 0) {
            continue;
        }
        copia = pCola[numCola];
        sem_wait(vacios[numCola]);
        sem_wait(mutex[numCola]);
        int i =0;
        while (copia->numeroMuestras != 0){
            dir0 = ((char*)copia) +sizeof(struct Examen);
            copia = (struct Examen*) dir0;
            i++;
            if(i == tamColasE){
                dir0 = pCola[numCola];
                i =0;
            }
        }
        id = id + rand() %max + 1;
        copia->tipo = tipo;
        copia->numeroMuestras = numMuestras;
        copia->numCola = numCola;
        copia->id = id;
        sem_post(mutex[numCola]);
        sem_post(llenos[numCola]);
        salida << id << endl;
    }
    int pos = 0;
    void *d;
    struct Examen*copias = pCola[0];
    for(int i = 0; i < 30; i++) {
            if(pos == 0){
                copias = pCola[0];
            } else {
                d= ((char*) copias) + sizeof(struct Examen);
                copias = (struct Examen *)((char *)d);
            }
            pos ++;
        }
    
    file.close();
    salida.close();
}

void Reg::interactivo(string nombreSeg){
    mode = true;
    cout << "> ";
    int numCola;
    char tipo;
    int numMuestras;

    void* dir0 = openMem(nombreSeg);
    struct Header *pHeader = (struct Header*) dir0;
    int numColasE = pHeader->numColasE;
    int tamColasE = pHeader->tamColasE;
    int tamColasS = pHeader->tamColasS;


    Examen *pCola[numColasE];
    void* dir[numColasE];
    dir[0] = ((char*)pHeader) + sizeof(struct Header);
    pCola[0] = (struct Examen*) dir[0];
    for(int i = 1;i < numColasE; i++) {
        dir[i] = ((char*)dir[i-1]) + sizeof(struct Examen) *  pHeader->tamColasE;
        pCola[i] = (struct Examen*) dir[i];
    }

    string nombre;
    sem_t *vacios[numColasE], *llenos[numColasE], *mutex[numColasE];

    for (int i = 0; i < numColasE ; i++ ){
        nombre =  nombreSeg + "-vacios" + to_string(i);
        vacios[i] = sem_open(nombre.c_str(), 0);
        nombre =  nombreSeg + "-llenos" + to_string(i);
        llenos[i] = sem_open(nombre.c_str(), 0);
        nombre =  nombreSeg + "-mutex" + to_string(i);
        mutex[i]  = sem_open(nombre.c_str(), 0);
    }

    struct Examen* copia;
    srand(time(NULL));

    string examen;
    int max = 2000 + numColasE *tamColasE + tamColasS;
    while(cin >> examen) {
        numCola = stoi(examen);
        cin >> examen;
        tipo = examen[0u];
        cin >> examen;
        numMuestras = stoi(examen);
        if( numMuestras < 0 || numMuestras >5) {
            cout << "> ";
            continue;
        } else  if(numCola > tamColasE - 1 || numCola < 0) {
            cout << "> ";
            continue;
        }

        copia = pCola[numCola];
        sem_wait(vacios[numCola]);
        sem_wait(mutex[numCola]);
        int i = 0;
        while (copia->numeroMuestras != 0){
            dir0 = ((char*)pHeader) +sizeof(struct Examen);
            copia = (struct Examen*)dir0 ;
            i++;
            if(i == tamColasE){
                dir0 = pCola[numCola];
                i =0;
            }
        }
        id = id + rand() %max + 1;
        copia->tipo = tipo;
        copia->numeroMuestras = numMuestras;
        copia->numCola = numCola;
        copia->id = id;
        sem_post(mutex[numCola]);
        sem_post(llenos[numCola]);
        cout << id << endl;       
        cout << "> ";
    }

}

void* Reg::openMem(string nombreSeg){
    int fd = shm_open(nombreSeg.c_str(),O_RDWR,0660);
    void* dir0;
    dir0 = mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    struct Header *pHeader = (struct Header*) dir0;
    int numColasE = pHeader->numColasE;
    int tamColasE = pHeader->tamColasE;
    int tamColasS = pHeader->tamColasS;

    munmap(dir0,sizeof(struct Header));
    dir0 = mmap(NULL,sizeof(struct Header)+ sizeof(struct Examen)*numColasE*tamColasE + sizeof(struct Examen)*tamColasS, PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    
    return dir0;
}