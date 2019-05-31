#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <pthread.h>
#include "elementos.h"
#include "init.h"

using namespace std;

Init in;
int main(void) {
    in.init(); 
}

void Init::init(void) {
    int tamColasE = 6;
    int tamColasS = 10;
    int numColasE = 5;
    int rBlood = 100;
    int rSkin = 100;
    int rDet = 100;     
    int tamColasI = 6;
    int fd;
    segName = "evaluator";
    blood.tamano = tamColasI;
    skin.tamano = tamColasI;
    detritus.tamano = tamColasI;

    fd = shm_open(segName.c_str(), O_RDWR | O_CREAT | O_EXCL, 0660); 
    if (fd < 0) {
        cerr << "Error creando la memoria compartida: " << errno << strerror(errno) << endl;
        exit(1);
    }
    // ftruncate amplia el segmento al tamaño que necesitamos
    if(ftruncate(fd,(sizeof(struct Header) + sizeof(struct Cola)*numColasE * tamColasE + sizeof(struct Cola) * tamColasS)) != 0) {
        cerr << "Error creando la memoria compartida" << errno << strerror(errno) << endl;
        exit(1);
    }

    struct Header *pHeader;
    void* dir0 = mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    pHeader = (struct Header*)dir0;
    pHeader->tamColasE = tamColasE;
    pHeader->numColasE = numColasE;
    pHeader->rBlood = rBlood;
    pHeader->rSkin = rSkin;
    pHeader->rDet = rDet; 

    Cola *pColasE[numColasE + 1];
    for(int i = 1; i < numColasE +2; i++) {
        if (i == numColasE + 1) {
            pColasE[i] = (struct Cola*) ((char*)dir0 + sizeof(struct Header) + (sizeof(struct Cola) * numColasE));
            pColasE[i]->tamano = tamColasS;
        } else {
            pColasE[i] = (struct Cola *) ((char*)dir0 + (sizeof(struct Cola) * (i-1)) +  sizeof(struct Header));
            pColasE[i]->tamano = tamColasE;
        }
    }
    close(fd);
    evaluador(numColasE, dir0);
}

void Init::evaluador(int cantidad, void* dir) {
    pthread_t entrada[cantidad];
    Cola *infoCola[cantidad];
    for (int i = 0; i < cantidad; i++) {
        infoCola[i] = (struct Cola *) ((char*)dir + (sizeof(struct Cola) * i) +  sizeof(struct Header));
        string name = "mutex" + to_string(i);
        infoCola[i]->mutex = sem_open(name.c_str(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR  | S_IRGRP | S_IWGRP, 0);
        pthread_create(&entrada[i], NULL, leerEntrada, (void*) infoCola[i]);
        sleep(1);
    }

    pthread_t thr_skin, thr_blood, thr_det;
    Cola *colaSalida = (struct Cola *) ((char*)dir + (sizeof(struct Cola) * cantidad) +  sizeof(struct Header));
    thread_args ta;
    ta.pCola = colaSalida;
    ta.tipo = skin;
    ta.nombreCola = "skin";
    ta.segName = segName;
    pthread_create(&thr_skin, NULL, procesar, (void *) &ta);
    sleep(1);
    ta.tipo = blood;
    ta.nombreCola = "blood";
    ta.segName = segName;
    pthread_create(&thr_blood, NULL, procesar, (void *) &ta);
    sleep(1);
    ta.tipo = detritus;
    ta.nombreCola = "detritus";
    ta.segName = segName;
    pthread_create(&thr_det, NULL, procesar, (void *) &ta);
    sleep(1);

    for (int i = 0; i < cantidad; i++) {
        pthread_join(entrada[i], NULL);
    }
    pthread_join(thr_blood, NULL);
    pthread_join(thr_skin, NULL);
    pthread_join(thr_det, NULL);

}

void* Init::leerEntrada(void *arg) { // Saca el examen de la bandeja de entrda y lo mete en lsa intermedias
    Cola *cInfo = (Cola*) arg;
    Examen ex;
    int i = 0;
    char tipoE;
    int random;
    srand (time(NULL));
    for(;;) {
        if(cInfo->cola.size() > 0) {
            sem_wait(cInfo->mutex);
            ex = cInfo->cola.back();
            tipoE = ex.tipo;
            random = rand() % 5 + 1; 
            ex.tiempo = random;
            switch(tipoE) {
                case 'B':
                    while(blood.cola.size() == blood.tamano) {
                        cout << "Esta llena" << endl;
                    };
                    blood.cola.insert(blood.cola.begin(),ex);
                    break;
                case 'S':
                    while(skin.cola.size() == blood.tamano);
                    skin.cola.insert(skin.cola.begin(),ex);
                    break;
                case 'D':
                    while(detritus.cola.size() == blood.tamano);
                    detritus.cola.insert(detritus.cola.begin(),ex);
                    break;
            }
            cInfo->cola.pop_back();
            //cout << "SIZE COLA  " << cInfo->cola.size() <<  "      " << endl;
            ++i;
        } else {
            break;
        }
    } 
} 

void* Init::procesar(void *arg) {
    thread_args *args = (thread_args*) arg;
    Cola *cInfo = args->pCola;
    Cola tipo = args->tipo;
    string nombreCola = args->nombreCola;
    string segName = args->segName;
    int fd = shm_open(segName.c_str(),O_RDWR,0660);
    void* dir = mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    struct Header *pHeader = (struct Header*)dir;
    srand (time(NULL)); // random seed
    for(;;) {
        cout << tipo.cola.size() << endl;
        if(tipo.cola.size() > 0) {
            Examen ex = tipo.cola.back();
            tipo.cola.pop_back();
            //cout << "Num Muestras " <<ex.numeroMuestras << endl;
            int random;
            while(cInfo->cola.size() == cInfo->tamano);
            for (int i = 0; i < ex.numeroMuestras; i++) {
                if(nombreCola == "skin") {
                    //cout << "--------SKIN-------" << endl; 
                    random = rand() % 7 + 1;
                    while(pHeader->rSkin - random <= 0);
                    pHeader->rSkin -= random;
                    //cout << pHeader->rSkin << endl;
                } else if(nombreCola == "blood") {
                    //cout << "--------BLOOD-------" << endl; 
                    random =  rand() % 20 + 5;
                    while(pHeader->rBlood - random <= 0); 
                    pHeader->rBlood -= random;
                    //cout << pHeader->rBlood << endl;
                } else {
                    //cout << "--------DET-------" << endl; 
                    random =  rand() % 25 + 8;
                    while(pHeader->rDet - random <= 0); 
                    pHeader->rDet -= random;
                    //cout << pHeader->rDet << endl;
                }
            }
            int tiempo = rand() % 5 + 1;
            ex.tiempo = tiempo;
            int resultado = rand() % 50 + 0;
            if(resultado <= 15) {
                ex.resultado = '?';
            } else if(resultado > 15 && resultado <= 35) {
                ex.resultado = 'n';
            } else {
                ex.resultado = 'p';
            }
            cInfo->cola.insert(cInfo->cola.begin(), ex);
        } else {
            break;
        }
    }
}
