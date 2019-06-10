#include "elementos.h"
#include "init.h"
#include "reg.h"

using namespace std;

sem_t *mutexI[3], *vaciosI[3], *llenosI[3];
sem_t *vaciosS, *llenosS, *mutexS;
static vector<Examen> blood, skin, detritus;
int maxBlood;
int id;
bool terminar = false;

Init::Init() {}
Init::~Init() {}

void Init::init(int i, int ie, int oe, string n, int b, int d, int s, int q) {
    int tamColasE = ie;
    int tamColasS = oe;
    int numColasE = i;
    int rBlood = b;
    int rSkin = s;
    int rDet = d;
    int tamColasI = q;
    segName = n;

    int maxBlood = b;
    int maxSkin = s;
    int maxDet = d;

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
    pHeader->maxBlood = maxBlood;
    pHeader->maxSkin = maxSkin;
    pHeader->maxDet = maxDet;
    close(fd);
    string nameColasI = "colasInt";
    int colasInt = shm_open(nameColasI.c_str(), O_RDWR | O_CREAT | O_EXCL, 0660);
    ftruncate(colasInt, sizeof(struct Examen)*tamColasI*3);
    void *dirI = mmap(NULL, (sizeof(struct Examen))*tamColasI*3, PROT_READ | PROT_WRITE, MAP_SHARED, colasInt, 0);
    close(colasInt);


    evaluador(pHeader, segName, dirI);
    //munmap(dir, sizeof(struct Header) + (sizeof(struct Examen) * numColasE * tamColasE) + (sizeof(struct Examen) * tamColasS));
}

void Init::evaluador(Header *pHeader, string segName, void *dirI) {
    string nombre;
    int numColasE = pHeader->numColasE;
    pthread_t entrada[numColasE];
    sem_t *mutex[numColasE];
    sem_t *vacios[numColasE];
    sem_t *llenos[numColasE];

    Examen *pCola[numColasE + 1];
    void *dir[numColasE];
    dir[0] = ((char *)pHeader) + sizeof(struct Header); // Direccion Header + Tamano Header
    pCola[0] = (struct Examen *)((char *)dir[0]);    //primera direccion inicio cola
    for (int i = 1; i < numColasE + 1; i++) {
        dir[i] = ((char *)dir[i - 1]) + (sizeof(struct Examen) * pHeader->tamColasE); // Siguiente direccion
        pCola[i] = (struct Examen *)((char *)dir[i]);                                 // Siguiente cola
    }

    Examen *pI[3];
    void *dirInt[3];
    dirInt[0] = ((char *)dirI); // Direccion primer examen
    pI[0] = (struct Examen *)((char *)dirInt[0]);    //primera direccion inicio cola

    for (int i = 1; i < 3; i++) {
        dirInt[i] = ((char *)dirInt[i - 1]) + (sizeof(struct Examen) * pHeader->tamColasI); // Siguiente direccion
        pI[i] = (struct Examen *)dirInt[i];   // Siguiente cola
    }

    for (int i = 0; i < numColasE; i++) {
        //Creacion semaforos para cada Cola !Menos la de salida!
        nombre = segName + "-mutex" + to_string(i);
        mutex[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 1);
        nombre = segName + "-vacios" + to_string(i);
        vacios[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, pHeader->tamColasE-1);
        nombre = segName + "-llenos" + to_string(i);
        llenos[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 0);
        pthread_info *pi = new pthread_info;
        pi->pCola = dir[i];
        pi->numHilo = i;
        pi->mutex = mutex[i];
        pi->llenos = llenos[i];
        pi->vacios = vacios[i];
        pi->tamano = pHeader->tamColasE;
        pi->pInt = pI[0];
        pi->tamColasI = pHeader->tamColasI;
        pi->i = 0;
        //Creacion hilo por cada cola
        pthread_create(&entrada[i], NULL, leerEntrada, pi);
        sleep(1);
    }

    char letra = 'B';
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            letra = 'S';
        } else if (i == 2) {
            letra = 'D';
        }
        nombre = segName + "mutexI" + letra;
        mutexI[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 1);
        nombre = segName + "vaciosI" + letra;
        vaciosI[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, pHeader->tamColasI-1);
        nombre = segName + "llenosI" + letra;
        llenosI[i] = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 0);
    }

    nombre = segName + "vaciosSR";
    vaciosS = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, pHeader->tamColasS-1);
    nombre = segName + "mutexSR";
    mutexS = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 1);
    nombre = segName + "llenosSR";
    llenosS = sem_open(nombre.c_str(), O_CREAT | O_EXCL, 0660, 0);

    pthread_t thr_skin, thr_blood, thr_det;

    pthread_info *piSalida1 = new pthread_info;
    piSalida1->llenos = llenosS;
    piSalida1->mutex = mutexS;
    piSalida1->vacios = vaciosS;
    piSalida1->pHeader = pHeader;
    piSalida1->pInt = pI[0];
    piSalida1->pCola = (struct Examen *)((char *)pCola[numColasE]);
    piSalida1->tamano = 0;
    pthread_create(&thr_blood, NULL, procesar, piSalida1);

    pthread_info *piSalida2 = new pthread_info;
    piSalida2->llenos = llenosS;
    piSalida2->mutex = mutexS;
    piSalida2->vacios = vaciosS;
    piSalida2->pHeader = pHeader;
    piSalida2->pCola = (struct Examen *)((char *)pCola[numColasE]);
    piSalida2->pInt = pI[0];
    piSalida2->tamano = 1;
    pthread_create(&thr_skin, NULL, procesar, piSalida2);

    pthread_info *piSalida = new pthread_info;
    piSalida->pCola = (struct Examen *)((char *)pCola[numColasE]);
    piSalida->llenos = llenosS;
    piSalida->mutex = mutexS;
    piSalida->vacios = vaciosS;
    piSalida->pHeader = pHeader;
    piSalida->pInt = pI[0];
    piSalida->tamano = 2;
    pthread_create(&thr_det, NULL, procesar, piSalida);

    for (int i = 0; i < numColasE; i++) {
        pthread_join(entrada[i], NULL);
    }
    pthread_join(thr_blood, NULL);
    pthread_join(thr_skin, NULL);
    pthread_join(thr_det, NULL);
}

void *Init::leerEntrada(void *arg) {
    pthread_info *args = (pthread_info *)arg; // Obtener argumentos
    void *dir0 = args->pCola;                 //obteniendo direccion de la cola Entrada
    void* dirI = args->pInt;
    struct Examen *ex = (struct Examen *)((char *)dir0);
    struct Examen *copia = ex;
    Examen examen;
    int i = args->i;
    int random;
    srand(time(NULL));
    void *dir;
    for (;;) {
        sem_wait(args->llenos);
        sem_wait(args->mutex);

        while (copia->numeroMuestras == 0) {
            if (i == 0) {
                copia = ex;
            } else {
                dir = ((char *)copia) + sizeof(struct Examen);
                copia = (struct Examen *)dir;
            }
            i++;
            if (i == args->tamano) {
                i = 0;
                copia = ex;
            }
        }
        examen.tipo = copia->tipo;
        examen.id = copia->id;
        examen.numeroMuestras = copia->numeroMuestras;
        examen.numCola = copia->numCola;
        random = rand() % 5 + 1;
        examen.tiempo = random;
        insertar(examen, dirI, args->tamColasI);
        copia->numeroMuestras = 0;

        sem_post(args->mutex);
        sem_post(args->vacios);

        i = 0;
    }
}

void Init::insertar(struct Examen examen, void* dirI, int tamColasI) {
    struct Examen *ex = (struct Examen *)((char *)dirI);
    struct Examen *copia = ex;
    if (examen.tipo == 'B') {
        sem_wait(vaciosI[0]);
        sem_wait(mutexI[0]);
        int i = 0;
        void* dir;
        while (copia->numeroMuestras != 0) {
            if (i == 0) {
                copia = ex;
            } else {
                dir = ((char *)copia) + sizeof(struct Examen);
                copia = (struct Examen *)dir;
            }
            i++;
            if (i == tamColasI) {
                i = 0;
                copia = ex;
            }
        } 
        copia->tipo = examen.tipo;
        copia->numeroMuestras = examen.numeroMuestras;
        copia->numCola = examen.numCola;
        copia->id = examen.id;
        sem_post(mutexI[0]);
        sem_post(llenosI[0]);
    } else if (examen.tipo == 'S') {
        sem_wait(vaciosI[1]);
        sem_wait(mutexI[1]);
        int i = 0;
        void* dir = ((char*)ex) + sizeof(struct Examen)*tamColasI;
        copia =  (struct Examen*)dir;
        while (copia->numeroMuestras != 0) {
            if (i == 0) {
                dir = ((char*)ex) + sizeof(struct Examen)*tamColasI;
                copia = (struct Examen*)dir;
            } else {
                dir = ((char *)copia) + sizeof(struct Examen);
                copia = (struct Examen *)dir;
            }
            i++;
            if (i == tamColasI) {
                i = 0;
                copia = (struct Examen*)((char*)ex) + (sizeof(struct Examen)*tamColasI);
            }
        }
        copia->tipo = examen.tipo;
        copia->numeroMuestras = examen.numeroMuestras;
        copia->numCola = examen.numCola;
        copia->id = examen.id;
        sem_post(mutexI[1]);
        sem_post(llenosI[1]);
    } else if (examen.tipo == 'D') {
        sem_wait(vaciosI[2]);
        sem_wait(mutexI[2]);
        int i = 0;
        void* dir = ((char*)ex) + (sizeof(struct Examen)*tamColasI)*2;
        copia = (struct Examen*)dir;
        while (copia->numeroMuestras != 0) {
            if (i == 0) {
                dir = ((char*)ex) + (sizeof(struct Examen)*tamColasI)*2;
                copia = (struct Examen*)dir;
            } else {
                dir = ((char *)copia) + sizeof(struct Examen);
                copia = (struct Examen *)dir;
            }
            i++;
            if (i == tamColasI) {
                i = 0;
                copia = (struct Examen*)((char*)ex) + (sizeof(struct Examen)*tamColasI)*2;
            }
        }
        copia->tipo = examen.tipo;
        copia->numeroMuestras = examen.numeroMuestras;
        copia->numCola = examen.numCola;
        copia->id = examen.id;
        sem_post(mutexI[2]);
        sem_post(llenosI[2]);
    }
}

void *Init::procesar(void *arg) {
    pthread_info *args = (pthread_info *)arg;
    struct Examen *colaSalida = (struct Examen *)((char *)args->pCola);
    struct Header *pHeader = args->pHeader;
    Examen ex;
    struct Examen *copia = colaSalida;
    void *d;
    for (;;) {
        if (args->tamano == 0) {
            ex = leerIntermedia(0, 7, 1, pHeader, args->pInt);
        } else if (args->tamano == 1) {
            ex = leerIntermedia(1, 25, 8, pHeader,args->pInt);
        } else {
            ex = leerIntermedia(2, 20, 5, pHeader,args->pInt);
        }

        sem_wait(vaciosS);
        sem_wait(mutexS);

        copia = colaSalida;
        while (copia->numeroMuestras != 0) {
            d = ((char *)copia) + sizeof(struct Examen);
            copia = (struct Examen *)d;
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

Examen Init::leerIntermedia(int tipo, int max, int min, struct Header *pHeader, void* dirI) {
    int random;
    Examen ex;
    srand(time(NULL));
    struct Examen* primeraI = (struct Examen*)dirI;
    sem_wait(llenosI[tipo]);
    sem_wait(mutexI[tipo]);
    struct Examen* copia = primeraI;
    if (tipo == 0) {
        int i = 0;
        void* dir = ((char*)primeraI);
        copia = (struct Examen*)dir;
        while (copia->numeroMuestras == 0) {
            if (i == 0) {
                dir = ((char*)primeraI);
                copia = (struct Examen*)dir;
            } else {
                dir = ((char *)copia) + sizeof(struct Examen);
                copia = (struct Examen *)dir;
            }
            i++;
            if (i == pHeader->tamColasI) {
                i = 0;
                copia = (struct Examen*)((char*)primeraI);
            }
        }
        ex.id = copia->id;
        ex.tipo = copia->tipo;
        ex.numeroMuestras = copia->numeroMuestras;
        ex.numCola = copia->numCola;
        copia->numeroMuestras = 0;
    } else if (tipo == 1) {
        int i = 0;
        void* dir = ((char*)primeraI) + (sizeof(struct Examen)*pHeader->tamColasI);
        copia = (struct Examen*)dir;
        while (copia->numeroMuestras == 0) {
            if (i == 0) {
                dir = ((char*)primeraI) + (sizeof(struct Examen)*pHeader->tamColasI);
                copia = (struct Examen*)dir;
            } else {
                dir = ((char *)copia) + sizeof(struct Examen);
                copia = (struct Examen *)dir;
            }
            i++;
            if (i == pHeader->tamColasI) {
                i = 0;
                copia = (struct Examen*)((char*)primeraI) + (sizeof(struct Examen)*pHeader->tamColasI);
            }
        }
        ex.id = copia->id;
        ex.tipo = copia->tipo;
        ex.numeroMuestras = copia->numeroMuestras;
        ex.numCola = copia->numCola;
        copia->numeroMuestras = 0;
    } else{
        int i = 0;
        void* dir = ((char*)primeraI) + (sizeof(struct Examen)*pHeader->tamColasI)*2;
        copia = (struct Examen*)dir;
        while (copia->numeroMuestras == 0) {
            if (i == 0) {
                dir = ((char*)primeraI) + (sizeof(struct Examen)*pHeader->tamColasI)*2;
                copia = (struct Examen*)dir;
            } else {
                dir = ((char *)copia) + sizeof(struct Examen);
                copia = (struct Examen *)dir;
            }
            i++;
            if (i == pHeader->tamColasI) {
                i = 0;
                copia = (struct Examen*)((char*)primeraI) + (sizeof(struct Examen)*pHeader->tamColasI)*2;
            }
        }
        ex.id = copia->id;
        ex.tipo = copia->tipo;
        ex.numeroMuestras = copia->numeroMuestras;
        ex.numCola = copia->numCola;
        copia->numeroMuestras = 0;
    }
    sem_post(mutexI[tipo]);
    sem_post(vaciosI[tipo]);
    for (int i = 0; i < ex.numeroMuestras; i++) {
        random = rand() % max + min;
        if (tipo == 0) {
            while (pHeader->rBlood - random < 0) {
            }
            pHeader->rBlood -= random;
        } else if (tipo == 1) {
            while (pHeader->rSkin - random < 0) {
            }
            pHeader->rSkin -= random;
        } else {
            while (pHeader->rDet - random < 0) {
            }
            pHeader->rDet -= random;
        }
    }

    random = rand() % 50 + 0;
    if (random <= 15) {
        ex.resultado = '?';
    } else if (random > 15 && random <= 35) {
        ex.resultado = 'N';
    } else {
        ex.resultado = 'P';
    }

    return ex;
}

sem_t *Init::getSemLlenos() {
    return llenosS;
}

sem_t *Init::getSemMutex() {
    return mutexS;
}

sem_t *Init::getSemVacios() {
    return vaciosS;
}
