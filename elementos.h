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
    int id;
    int tiempo;
    char resultado;
};