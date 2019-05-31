#include <vector>
#include<semaphore.h>
#pragma once


struct Header{
    int rBlood;
    int rDet;
    int rSkin;
    int numColasE;
    int tamColasE;
    int tamColasS;
};

struct Examen{
    int numCola;
    char tipo;
    int numeroMuestras;
    //int id;
};

struct Cola {
    int cantidad;
    std::vector <Examen> cola;
    sem_t *mutex;
};