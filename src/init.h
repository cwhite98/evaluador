#pragma once
#include <string>
#include <vector>
#include "elementos.h"
#include <semaphore.h>


using namespace std;

class Init {
    public: 
        Init();
        ~Init();
        void init(int,int,int,string,int,int,int,int);
        void evaluador(struct Header *pH, string segName,void*);
        static void insertar(struct Examen);
        static void* leerEntrada(void *arg);
        static void* procesar(void *arg);
        int* getId();
        sem_t* getSemLlenos();
        sem_t* getSemMutex();
        static Examen leerIntermedia(int tipo, vector<Examen>*q,int max, int min, struct Header *pHeader);
    
    private:
    string segName;
};
struct pthread_info{
        void *pCola;
        int numHilo;
        sem_t *mutex;
        sem_t *vacios;
        sem_t *llenos;
        int tamano;
        int i;
        struct Header *pHeader;
};