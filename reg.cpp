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
#include <fstream>

using namespace std;

Reg::Reg() : id(0) {}
Reg::~Reg() {}

void Reg::leerArchivo(string archivo, string nombreSeg) { 
    ifstream file;
    file.open (archivo);
    string line;

    string nombre = archivo.substr (0,archivo.find('.'));
    nombre = nombre + ".spl";
    ofstream salida;
    salida.open(nombre);

    int fd = shm_open(nombreSeg.c_str(),O_RDWR,0660);
    void* dir0;
    dir0 = mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    struct Header *pHeader = (struct Header*) dir0;
    int numColasE = pHeader->numColasE;
    int tamColasE = pHeader->tamColasE;
    int tamColasS = pHeader->tamColasS;

    munmap(dir0,sizeof(struct Header));
    dir0 = mmap(NULL,sizeof(struct Header)+ sizeof(struct Examen)*numColasE*tamColasE + sizeof(struct Examen)*tamColasS, PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    Examen *pCola[numColasE];
    void* dir = ((char*)dir0) + sizeof(struct Header);
    pCola[0] = (struct Examen*) dir;
    void *dir1;
    for(int i = 1;i < numColasE; i++) {
        dir1 = ((char*)dir) + sizeof(struct Examen) *  pHeader->tamColasE;
        pCola[i] = (struct Examen*) dir1;
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
    while (file >> word) {
        numCola = stoi(word);
        file >> word;
        tipo = word[0u];
        file >> word;
        numMuestras = stoi(word);
        cout << "esta vacio?" << endl;
        copia = pCola[numCola-1];
        sem_wait(vacios[numCola -1]);
        sem_wait(mutex[numCola -1]);
        while (copia->numeroMuestras != 0) copia = (struct Examen*) ((char*)copia) +sizeof(struct Examen);
        copia->tipo = tipo;
        copia->numeroMuestras = numMuestras;
        copia->numCola = numCola;
        copia->id = id;
        sem_post(mutex[numCola -1]);
        sem_post(llenos[numCola -1]);
        id++;
    }
    
    file.close();
    salida.close();
}

void Reg::interactivo(string nombreSeg){

}