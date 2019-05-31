#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include "elementos.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <pthread.h>

using namespace std;

void* procesar(void*);
void* leerEntrada(void*);
void evaluador(int cantidad, void* dir[]);
void ingresar(Examen examen, Cola cSalida);
void *procesar(void *arg);
struct thread_args {
    Cola *pCola;
    Cola tipo;
    string nombreCola;
};

struct Cola skin, blood, detritus;
string segName;

int main(void) {
    int tamColasE = 6;
    int tamColasS = 10;
    int numColasE = 5;
    int rBlood = 100;
    int rSkin = 100;
    int rDet = 100;     
    int tamColasI = 6;
    int fd;
    segName = "evaluator";

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
    void *dirr[numColasE + 2];
    dirr[0]= mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    pHeader = (struct Header*)dirr[0];
    pHeader->tamColasE = tamColasE;
    pHeader->numColasE = numColasE;
    pHeader->rBlood = rBlood;
    pHeader->rSkin = rSkin;
    pHeader->rDet = rDet; 

    Cola *pColasE[numColasE + 1];
    for(int i = 1; i < numColasE + 2; i++) {
        dirr[i] = mmap(NULL,sizeof(struct Cola), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (i == 2) {
            pColasE[i] = (struct Cola*)dirr[i] + sizeof(struct Header);
        /*} if (i == numColasE){
           pColasE[i] = (struct Cola*)dirr[i] +sizeof(struct Cola); 
           pColasE[i]->cola.resize(10);*/
        } else {
            pColasE[i] = (struct Cola*)dirr[i] + (sizeof(struct Cola) * i) +  sizeof(struct Header); 
        }
        //cout << i << "      " << pColasE[i]->cola.size() << endl;
    }
    //cout << 0 << "      " << pColasE[0]->cola.size() << endl;

    close(fd);
    evaluador(numColasE, dirr);
}

void evaluador(int cantidad, void* dir[]) {
    pthread_t entrada[cantidad];
    Cola *infoCola[cantidad];
    for (int i = 0; i < cantidad; i++) {
        infoCola[i] = (struct Cola *) dir[i+1] + (sizeof(struct Cola) * i) +  sizeof(struct Header);
        string name = "mutex" + to_string(i);
        infoCola[i]->mutex = sem_open(name.c_str(), O_CREAT | O_EXCL, S_IRUSR | S_IWUSR  | S_IRGRP | S_IWGRP, 0);
        Examen e= {i,'B',2};
        infoCola[i]->cola.insert(infoCola[i]->cola.begin(),e);
        Examen a = {i,'S',9};
        infoCola[i]->cola.insert(infoCola[i]->cola.begin(),a);
        pthread_create(&entrada[i], NULL, leerEntrada, (void *) infoCola[i]);
        sleep(1);
    }


    pthread_t thr_skin, thr_blood, thr_det;
    Cola *colaSalida = (struct Cola *) dir[0] + (sizeof(struct Cola) * cantidad) +  sizeof(struct Header);
    thread_args ta;
    ta.pCola = colaSalida;
    ta.tipo = skin;
    ta.nombreCola = "skin";
    pthread_create(&thr_skin, NULL, procesar, (void *) &ta);
    sleep(1);
    ta.tipo = blood;
    ta.nombreCola = "blood";
    pthread_create(&thr_blood, NULL, procesar, (void *) &ta);
    sleep(1);
    ta.tipo = detritus;
    ta.nombreCola = "detritus";
    pthread_create(&thr_det, NULL, procesar, (void *) &ta);
    sleep(1);

    for (int i = 0; i < cantidad; i++) {
        pthread_join(entrada[i], NULL);
    }
    pthread_join(thr_blood, NULL);
    pthread_join(thr_skin, NULL);
    pthread_join(thr_det, NULL);

}

void* leerEntrada(void *arg) { // Saca el examen de la bandeja de entrda y lo mete en lsa intermedias
    Cola *cInfo = (Cola*) arg;
    //cout << "ENTRE" << endl;
    Examen ex;
    int i = 0;
    char tipoE;
    for(;;) {
        if(cInfo->cola.size() > 0) {
            sem_wait(cInfo->mutex);
            cout << "-----" << i << endl;
            ex = cInfo->cola.back();
            tipoE = ex.tipo;
            switch(tipoE) {
                case 'B':
                    blood.cola.insert(blood.cola.begin(),ex);
                    break;
                case 'S':
                    skin.cola.insert(skin.cola.begin(),ex);
                    break;
                case 'D':
                    detritus.cola.insert(detritus.cola.begin(),ex);
                    break;
            }
            cInfo->cola.pop_back();
            cout << "SIZE COLA  " << cInfo->cola.size() <<  "      " << endl;
            ++i;
        } else {
            break;
        }
    } 
} 

void *procesar(void *arg) {
    thread_args *args = (thread_args*) arg;
    Cola *cInfo = args->pCola;
    Cola tipo = args->tipo;
    string nombreCola = args->nombreCola;
    int fd = shm_open(segName.c_str(),O_RDWR,0660);
    void* dir = mmap(NULL,sizeof(struct Header), PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);
    struct Header *pHeader = (struct Header*)dir;
    srand (time(NULL)); // random seed
    for(;;) {
        if(tipo.cola.size() > 0) {
            cout << " entre al if" << endl;
            Examen ex = tipo.cola.back();
            tipo.cola.pop_back();
            cout << "Num Muestras " <<ex.numeroMuestras << endl;
            int random;
            for (int i = 0; i < ex.numeroMuestras; i++) {
                if(nombreCola == "skin") {
                    cout << "--------SKIN-------" << endl; 
                    random =  rand() % 7 + 1;
                    if( pHeader->rSkin - random > 0) {
                        pHeader->rSkin -= random;
                        cout << pHeader->rSkin << endl;
                    } else {
                        cout << "Es 0" << endl;
                        // nada
                    }
                } else if(nombreCola == "blood") {
                    cout << "--------BLOOD-------" << endl; 
                    random =  rand() % 20 + 5;
                    if( pHeader->rBlood - random > 0) {
                        pHeader->rBlood -= random;
                        cout << pHeader->rBlood << endl;
                    } else {
                        cout << "Es 0" << endl;
                        // nada
                    }
                } else {
                    cout << "--------DET-------" << endl; 
                    random =  rand() % 25 + 8;
                    if( pHeader->rDet - random > 0) {
                        pHeader->rDet -= random;
                        cout << pHeader->rDet << endl;
                    } else {
                        cout << "Es 0" << endl;
                        // nada
                    }
                }
            }
        } else {
            break;
        }
    }
}
