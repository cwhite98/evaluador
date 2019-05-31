#pragma once

#include "elementos.h"
#include <string>
#include <cstring>

using namespace std;

class Init {
    public: 
        void init(void);
        void evaluador(int cantidad, void* dir);
        static void* leerEntrada(void *arg);
        void ingresar(Examen examen, Cola cSalida);
        static void* procesar(void *arg);
    private:
        struct thread_args {
            Cola *pCola;
            Cola tipo;
            string nombreCola;
            string segName;
        };
        string segName;
};