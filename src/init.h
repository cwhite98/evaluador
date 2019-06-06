#pragma once
#include <string>
#include <queue>
#include "elementos.h"
#include <semaphore.h>


using namespace std;

class Init {
    public: 
        Init();
        ~Init();
        void init(int,int,int,string,int,int,int,int);
        void evaluador(struct Header *pH, string segName);
        static void* leerEntrada(void *arg);
         static void* procesar(void *arg);
    
    private:
    string segName;
};
struct pthread_info{
        Examen *pCola;
        sem_t *mutex;
        sem_t *vacios;
        sem_t *llenos;
        int tamano;
};