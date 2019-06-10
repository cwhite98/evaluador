#pragma once
#include <string>

struct Header{
    int rBlood;
    int rDet;
    int rSkin;
    int numColasE;
    int tamColasE;
    int tamColasS;
    int tamColasI;
    int maxBlood;
    int maxSkin;
    int maxDet;
};

struct Examen{
    int numCola;
    char tipo;
    int numeroMuestras;
    int id;
    int tiempo;
    char resultado;
};
