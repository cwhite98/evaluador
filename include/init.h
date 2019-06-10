#pragma once
#include <string>
#include <vector>
#include "elementos.h"
#include <semaphore.h>
#include <iomanip>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <pthread.h>


using namespace std;

class Init {
    public: 
        Init();
        ~Init();
        void init(int,int,int,string,int,int,int,int);
        void evaluador(struct Header *pH, string segName, void*);
        static void insertar(struct Examen, void*, int);
        static void* leerEntrada(void *arg);
        static void* procesar(void *arg);
        sem_t* getSemLlenos();
        sem_t* getSemMutex();
        sem_t* getSemVacios();
        int getMaxBlood();
        int getMaxSkin();
        int getMaxDet();
        void getHilos(int);
        static Examen leerIntermedia(int tipo,int max, int min, struct Header *pHeader,void*);
    
    private:
    string segName;
    vector <sem_t*> sems;
    int maxBlood, maxDet, maxSkin;
};
struct pthread_info{
        void *pCola;
        int numHilo;
        sem_t *mutex;
        sem_t *vacios;
        sem_t *llenos;
        int tamano;
        int i;
        int tamColasI;
        struct Header *pHeader;
        void* pInt;
};
extern bool terminar;
//vector<Examen> blood, skin, detritus;
