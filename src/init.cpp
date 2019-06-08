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
#include "elementos.h"
#include "init.h"
#include <iomanip>
#include <sstream>

using namespace std;

Init::Init() {}
Init::~Init() {}
sem_t *mutexI[3], *vaciosI[3], *llenosI[3];
sem_t *vaciosS, *llenosS, *mutexS;
vector<Examen> blood, skin, detritus;
int id;

void Init::init(int i, int ie, int oe, string n, int b, int d, int s, int q) {
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
    ftruncate(fd, sizeof(struct Header) + (sizeof(struct Examen) * numColasE * tamColasE) + (sizeof(struct Examen) * tamColasS));
    void *dir = mmap(NULL, sizeof(struct Header) + ((sizeof(struct Examen) * numColasE * tamColasE) + ((sizeof(struct Examen) * tamColasS))), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   struct Header *pHeader = (struct Header *)dir;
    pHeader->tamColasE = tamColasE;
    pHeader->tamColasI = tamColasI;
    pHeader->numColasE = numColasE;
    pHeader->rBlood = rBlood;
    pHeader->rSkin = rSkin;
    pHeader->rDet = rDet;
    pHeader->tamColasS = tamColasS;
    close(fd);
    
    evaluador(pHeader, segName,dir);
    munmap(dir, sizeof(struct Header) + (sizeof(struct Examen) * numColasE * tamColasE) + (sizeof(struct Examen) * tamColasS));
}

void Init::evaluador(Header *pHeader, string segName, void* dir0) {
    cout << "Evaluador " << endl;
    string nombre;
    int numColasE = pHeader->numColasE;
    pthread_t entrada[numColasE];

    sem_t *mutex[numColasE];
    sem_t *vacios[numColasE];
    sem_t *llenos[numColasE];

/*Prueba  */
    
    Examen *pCola[numColasE+1];
    void* dir[numColasE];
    dir[0] = ((char*)dir0) + sizeof(struct Header); // Direccion Header + Tamano Header
    pCola[0] = (struct Examen*) ((char*)dir[0]); //primera direccion inicio cola
    for(int i = 1;i < numColasE+1; i++) {
        dir[i] = ((char*)dir[i-1]) + (sizeof(struct Examen) *  pHeader->tamColasE); // Siguiente direccion 
        pCola[i] = (struct Examen*) ((char *)dir[i]); // Siguiente cola
    }


    for (int i = 0; i < numColasE; i++) {
        //Creacion semaforos para cada Cola !Menos la de salida!
        nombre = segName + "-mutex" + to_string(i);
        mutex[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 1);
        nombre = segName + "-vacios" + to_string(i);
        vacios[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, pHeader->tamColasE - 1);
        nombre = segName + "-llenos" + to_string(i);
        llenos[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 0);
        pthread_info* pi = new pthread_info;
        pi->pCola=dir[i];
        pi->numHilo = i;
        pi->mutex = mutex[i];
        pi->llenos = llenos[i];
        pi->vacios = vacios[i];
        pi->tamano = pHeader->tamColasE;
        pi->i = 0;
        //Creacion hilo por cada cola
        pthread_create(&entrada[i], NULL, leerEntrada,pi);
        sleep(1);
    }

    char letra = 'B';
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            letra = 'S';
        }
        else if (i == 2) {
            letra = 'D';
        }
        nombre = segName + "mutex" + letra;
        mutexI[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 1);
        nombre = segName + "vacios" + letra;
        vaciosI[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, pHeader->tamColasI - 1);
        nombre = segName + "llenos" + letra;
        llenosI[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 0);
    }

    nombre = segName + "vaciosS";
    vaciosS = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, pHeader->tamColasS-1);
    nombre = segName + "mutexS";
    mutexS = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 1);
    nombre = segName + "llenosS";
    llenosS = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 0);

    pthread_t thr_skin, thr_blood, thr_det;

    pthread_info *piSalida1 = new pthread_info;
    piSalida1->llenos = llenosS;
    piSalida1->mutex = mutexS;
    piSalida1->vacios = vaciosS;
    piSalida1->pHeader = pHeader;
    piSalida1->pCola = (struct Examen *)((char *)pCola[numColasE]);
    piSalida1->tamano = 0;
    pthread_create(&thr_blood, NULL, procesar, piSalida1);

    pthread_info *piSalida2 = new pthread_info; 
    piSalida2->llenos = llenosS;
    piSalida2->mutex = mutexS;
    piSalida2->vacios = vaciosS;
    piSalida2->pHeader = pHeader;
    piSalida2->pCola = (struct Examen *)((char *)pCola[numColasE]);
    piSalida2->tamano = 1;
    pthread_create(&thr_skin, NULL, procesar, piSalida2);

    pthread_info *piSalida = new pthread_info;
    piSalida->pCola = (struct Examen *)((char *)pCola[numColasE]);
    piSalida->llenos = llenosS;
    piSalida->mutex = mutexS;
    piSalida->vacios = vaciosS;
    piSalida->pHeader = pHeader;
    piSalida->tamano = 2;
    pthread_create(&thr_det, NULL, procesar, piSalida);

    for (int i = 0; i < numColasE; i++) {
        pthread_join(entrada[i], NULL);
    }
    pthread_join(thr_blood, NULL);
    pthread_join(thr_skin, NULL);
    pthread_join(thr_det, NULL);
/*Fin prueba */

}

void *Init::leerEntrada(void *arg) {
    cout << "Leer entrada " << endl;
    pthread_info *args = (pthread_info *)arg; // Obtener argumentos
    void *dir = args->pCola; //obteniendo direccion de la cola
    cout << "Direccion cola anterior " << dir << endl;
    struct Examen *ex = (struct Examen*) ((char*) dir);
    struct Examen* copia = ex;
    Examen examen;
    int i = args->i;
    int random;
    srand(time(NULL));

    for (;;) {
        cout << "HAY? " << endl;
        sem_wait(args->llenos);
        cout << " Si Hay" << endl;
        sem_wait(args->mutex);
         if (i == 0) {
            copia = ex;
        } else if (i == args->tamano) {
            i = 0;
            copia = ex;
        } else {
            copia = (struct Examen*) ((char*)copia) +sizeof(struct Examen);
        }
        if (copia->numeroMuestras == 0) {
            i = 0;
            copia = ex;
        }
        examen.tipo = copia->tipo;
        examen.id = copia->id;
        cout << "Num muestras " << copia->numeroMuestras << endl;
        examen.numeroMuestras = copia->numeroMuestras;
        examen.numCola = copia->numCola;
        random = rand() % 5 + 1;
        examen.tiempo = random;
        cout << copia->tipo << endl;
        insertar(examen);
        //Examen * res = copia;
        copia->numeroMuestras = 0;
        i++;

        sem_post(args->mutex);
        sem_post(args->vacios);
        sleep(1);
    }
}

void Init::insertar(struct Examen examen) {
    cout << "insertar " << endl;
    if (examen.tipo == 'B') {
        sem_wait(vaciosI[0]);
        sem_wait(mutexI[0]);
        blood.insert(blood.begin(), examen);
        cout << "Inserte B " << blood.size() << endl;
        cout << "NMB " <<blood.back().numeroMuestras << endl;
        sem_post(mutexI[0]);
        sem_post(llenosI[0]);
    }
    else if (examen.tipo == 'S') {
        sem_wait(vaciosI[1]);
        sem_wait(mutexI[1]);
        skin.insert(skin.begin(), examen);
        cout << "Inserte S " << skin.size() << endl;
        cout << "NMS" <<skin.back().numeroMuestras << endl;
        sem_post(mutexI[1]);
        sem_post(llenosI[1]);
    }
    else if (examen.tipo == 'D') {
        sem_wait(vaciosI[2]);
        sem_wait(mutexI[2]);
        detritus.insert(detritus.begin(), examen);
         cout << "Inserte D " << detritus.size() << endl;
        cout << "NMD " <<detritus.back().numeroMuestras << endl;
        sem_post(mutexI[2]);
        sem_post(llenosI[2]);
    }
}

 void *Init::procesar(void *arg) {
    cout << "procesar" << endl;
    pthread_info *args = (pthread_info *)arg;
    struct Examen *colaSalida = (struct Examen*) ((char*)args->pCola);
    struct Header *pHeader = args->pHeader;
    Examen ex;
    struct Examen *copia = colaSalida;
    void* d;
    for (;;) {
        if (args->tamano == 0) {
            cout << "soy blood" << endl;
            ex = leerIntermedia(0, &blood, 7, 1, pHeader);
            cout << "Blood resultado" << ex.resultado << endl;
        }
        else if (args->tamano == 1) {
            cout << "soy skin" << endl;
            ex = leerIntermedia(1, &skin, 25, 8, pHeader);
            cout << "Skin resultado" << ex.resultado << endl;
        }
        else {
            cout << "soy detritus" << endl;
            ex = leerIntermedia(2, &detritus, 20, 5, pHeader);
            cout << "Det resultado" << ex.resultado << endl;
        }
        sem_wait(vaciosS);
        sem_wait(mutexS);

        cout << "ANTES deL WHILE" << endl;
        copia = colaSalida;
        while (copia->numeroMuestras != 0) {
            d = ((char *)copia) + sizeof(struct Examen) ;
            copia = (struct Examen*) d;
        }

        copia->id = ex.id;
        copia->resultado = ex.resultado;
        copia->numCola = ex.numCola;
        copia->numeroMuestras = ex.numeroMuestras;
        copia->tiempo = ex.tiempo;
        copia->tipo = ex.tipo;


        sem_post(mutexS);
        sem_post(llenosS);
    }
}

Examen Init::leerIntermedia(int tipo, vector<Examen> *q, int max, int min, struct Header *pHeader) {
    int random;
    Examen ex;
    srand(time(NULL));
    cout << "Leyendo colas intermedias " << endl;
    sem_wait(llenosI[tipo]);
    cout << "Paso " << endl;
    sem_wait(mutexI[tipo]);
    cout << "Pase mutex" << endl;
    if (tipo == 0) {
        ex = blood.back();
        cout << "Numero meustras leerI Blood " <<ex.numeroMuestras << endl;
        cout << ex.id << endl;
        blood.pop_back();
    }
    else if (tipo == 1) {
        ex = skin.back();
        skin.pop_back();
         cout << "Numero meustras leerI Skin " <<ex.numeroMuestras << endl;
         cout << ex.id << endl;
    }
    else {
        ex = detritus.back();
        detritus.pop_back();
        cout << "Numero meustras leerI Detr " <<ex.numeroMuestras << endl;
        cout << ex.id << endl;
    }
    for (int i = 0; i < ex.numeroMuestras; i++) {
        random = rand() % max + min;
        if (tipo == 0) {
            while (pHeader->rBlood - random <= 0);
            pHeader->rBlood -= random;
        }
        else if (tipo == 1) {
            while (pHeader->rSkin - random <= 0);
            pHeader->rSkin -= random;
        }
        else {
            while (pHeader->rDet - random <= 0);
            pHeader->rDet -= random;
        }
    }

    random = rand() % 50 + 0;
    if (random <= 15) {
        ex.resultado = '?';
    }
    else if (random > 15 && random <= 35) {
        ex.resultado = 'N';
    }
    else {
        ex.resultado = 'P';
    }

    sem_post(mutexI[tipo]);
    sem_post(vaciosI[tipo]);

    return ex;
}

sem_t* Init::getSemLlenos() {
   return llenosS;
}

sem_t* Init::getSemMutex(){
    return mutexS;
}