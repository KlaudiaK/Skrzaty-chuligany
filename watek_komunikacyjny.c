#include "main.h"
#include "watek_komunikacyjny.h"


/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr) {
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;

    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (stan != InFinish) {
        debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        sem_wait(&l_clock_sem);
        if (pakiet.ts > l_clock) {
            l_clock = pakiet.ts + 1;
        } else {
            l_clock++;
        }
        sem_post(&l_clock_sem);

        switch ( status.MPI_TAG ) {
            case REQ_EYE:
                println("Dostałem EYE_REQ od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack", status.MPI_SOURCE, nEye);
                pthread_mutex_lock(&mutex);
                insert(&eyeRequestQueue, status.MPI_SOURCE, pakiet.ts);
                sort(&eyeRequestQueue);
                if (stan == FREE) {
                    if(nEye > 0){
                    println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack", status.MPI_SOURCE);
                    sendPacket( 0, status.MPI_SOURCE, ACK_EYE );
                        nEye--;
                        removeNode(&eyeRequestQueue, status.MPI_SOURCE);
                    }
                } else if (stan == WAITING_FOR_EYE_AND_GUNPOINT ) {
                    if (isElementAmongFirst(eyeRequestQueue, status.MPI_SOURCE, nEye) && nEye > 0) {
                        println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                                status.MPI_SOURCE);
                        sendPacket(0, status.MPI_SOURCE, ACK_EYE);
                        nEye--;
                        removeNode(&eyeRequestQueue, status.MPI_SOURCE);
                    } else {
                        println("Proces %d nie może dostać pozwolenia na korzystanie z zasobu.", status.MPI_SOURCE);
                    }
                } else if (stan == PRODUCING_GUN) {
                    if (nEye > 1) {
                        println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                                status.MPI_SOURCE);
                        sendPacket(0, status.MPI_SOURCE, ACK_EYE);
                        nEye--;
                        removeNode(&eyeRequestQueue, status.MPI_SOURCE);
                    } else {
                        println("Proces %d nie może dostać pozwolenia na korzystanie z zasobu.", status.MPI_SOURCE);
                    }
                } else {
                    if(nEye > 0){
                    println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack", status.MPI_SOURCE);
                        sendPacket( 0, status.MPI_SOURCE, ACK_EYE );
                        nEye--;
                        removeNode(&eyeRequestQueue, status.MPI_SOURCE);
                        }
                }
                pthread_mutex_unlock(&mutex);
                break;
            case REQ_GP:
                println("Dostałem GP_REQ od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack",status.MPI_SOURCE, nGunpoint);
                pthread_mutex_lock(&mutex);
                insert(&gPRequestQueue, status.MPI_SOURCE, pakiet.ts);
                sort(&gPRequestQueue);
                if (stan == FREE) {
                    if (nGunpoint > 0) {
                    println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                            status.MPI_SOURCE);
                    sendPacket(0, status.MPI_SOURCE, ACK_GP);
                    removeNode(&gPRequestQueue, status.MPI_SOURCE);
                    nGunpoint--;
                    }
                } else if (stan == WAITING_FOR_EYE_AND_GUNPOINT) {
                    if (isElementAmongFirst(gPRequestQueue, status.MPI_SOURCE, nGunpoint) && nGunpoint > 0) {
                        println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                                status.MPI_SOURCE);
                        sendPacket(0, status.MPI_SOURCE, ACK_GP);
                        removeNode(&gPRequestQueue, status.MPI_SOURCE);
                        nGunpoint--;
                    } else {
                        println("Proces %d nie może dostać pozwolenia na korzystanie z zasobu.", status.MPI_SOURCE);
                    }
                } else if (stan == PRODUCING_GUN) {
                    if (nGunpoint > 1) {
                        println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                                status.MPI_SOURCE);
                        sendPacket(0, status.MPI_SOURCE, ACK_GP);
                        removeNode(&gPRequestQueue, status.MPI_SOURCE);
                        nGunpoint--;
                    } else {
                        println("Proces %d nie może dostać pozwolenia na korzystanie z zasobu.", status.MPI_SOURCE);
                    }
                } else {
                     if(nGunpoint > 0) {
                    println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                                status.MPI_SOURCE);
                        sendPacket(0, status.MPI_SOURCE, ACK_GP);
                         removeNode(&gPRequestQueue, status.MPI_SOURCE);
                        nGunpoint--;
                     }
                }
                pthread_mutex_unlock(&mutex);
                break;
            case REQ_GUN:
                println("Dostałem GUN_REQ od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack", status.MPI_SOURCE, nGun);
                pthread_mutex_lock(&mutex);
                insert(&gunRequestQueue, status.MPI_SOURCE, pakiet.ts);
                sort(&gunRequestQueue);
                if (stan == FREE) {
                    if(nGun > 0) {
                    println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                            status.MPI_SOURCE);
                    sendPacket(0, status.MPI_SOURCE, ACK_GUN);
                    removeNode(&gunRequestQueue, status.MPI_SOURCE);
                    nGun--;
                    }
                } else if (stan == WAITING_FOR_GUN) {
                    if (isElementAmongFirst(gunRequestQueue, status.MPI_SOURCE, nGun) && nGun > 0) {
                        println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                                status.MPI_SOURCE);
                        sendPacket(0, status.MPI_SOURCE, ACK_GUN);
                        removeNode(&gunRequestQueue, status.MPI_SOURCE);
                        nGun--;
                    } else {
                        println("Proces %d nie może dostać pozwolenia na korzystanie z zasobu.", status.MPI_SOURCE);
                    }
                } else if (stan == KILLING_RAT) {
                    if (nGun > 1) {
                        println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                                status.MPI_SOURCE);
                        sendPacket(0, status.MPI_SOURCE, ACK_GUN);
                        removeNode(&gunRequestQueue, status.MPI_SOURCE);
                        nGun--;
                    } else {
                        println("Proces %d nie może dostać pozwolenia na korzystanie z zasobu.", status.MPI_SOURCE);
                    }
                } else {
                     if(nGun > 0) {
                    println("Proces %d może dostać pozwolenie na korzystanie z zasobu, wysyłam ack",
                                status.MPI_SOURCE);
                        sendPacket(0, status.MPI_SOURCE, ACK_GUN);
                         removeNode(&gunRequestQueue, status.MPI_SOURCE);
                        nGun--;
                     }
                }
                pthread_mutex_unlock(&mutex);
                break;
            case ACK_EYE:
                debug("Dostałem ACK_EYE od %d, mam już %d", status.MPI_SOURCE, ackCountEye);
                pthread_mutex_lock(&mutex);
                ackCountEye++;
                if (ackCountEye == size - 1 && isElementAmongFirst(eyeRequestQueue, rank, nEye) == 1
                ) {
                    pthread_cond_signal(&condition);
                }
                pthread_mutex_unlock(&mutex);
                break;
            case ACK_GUN:
                pthread_mutex_lock(&mutex);
                ackCountGun++;
                debug("Dostałem ACK_GUN od %d, mam już %d", status.MPI_SOURCE, ackCountGun);
                println("dostepnosc broni to %d", nGun);
                printList(gunRequestQueue);

                println("Liczba ack GUN %d, jestem pierwszy %d", ackCountGun, isElementAmongFirst(gunRequestQueue, rank, nGun) == 1);

                if (ackCountGun == size - 1
                && isElementAmongFirst(gunRequestQueue, rank, nGun) == 1
                ){
                    pthread_cond_signal(&condition);
                    println("Wyslalem syyyyyyyyyyyygnal");
                }
                pthread_mutex_unlock(&mutex);
                break;
            case ACK_GP:
                debug("Dostałem ACK_GP od %d, mam już %d", status.MPI_SOURCE, ackCountGp);
                pthread_mutex_lock(&mutex);
                ackCountGp++;
                if (ackCountGp == size - 1 &&  isElementAmongFirst(gPRequestQueue, rank, nGunpoint) == 1) {
                    pthread_cond_signal(&condition);
                }
                pthread_mutex_unlock(&mutex);
                break;
            case RELEASE_GUN:
                debug("Dostałem RELEASE_GUN od %d, zwiekszam zasoby agrafek i celownikow", status.MPI_SOURCE);
                pthread_mutex_lock(&mutex);
                nEye++;
                nGunpoint++;
                removeNode(&gunRequestQueue, status.MPI_SOURCE);
                sort(&gunRequestQueue);
                sort(&gPRequestQueue);
                sort(&eyeRequestQueue);
                struct pair_id_ts* eyeReqQueueHead = eyeRequestQueue;
                int count_eye = 0;
                while (eyeReqQueueHead != NULL && count_eye < nEye) {
                    if (eyeReqQueueHead->id == rank)
                    {
                        if (ackCountEye == size - 1
                        && isElementAmongFirst(eyeRequestQueue, rank, nEye) == 1
                        ){
                            pthread_cond_signal(&condition);
                        }
                    } else {
                        sendPacket(0, eyeReqQueueHead->id, ACK_EYE);
                        println("Wysyłam ACK_EYE do procesu o miejscu w kolejce: %d", eyeReqQueueHead->id);
                        nEye--;
                    }
                    eyeReqQueueHead = eyeReqQueueHead->next;
                    count_eye++;
                }
                struct pair_id_ts* gpReqQueueHead = gPRequestQueue;
                int count_gp = 0;
                while (gpReqQueueHead != NULL && count_gp < nGunpoint) {
                    if (gpReqQueueHead->id == rank)
                    {
                        if (ackCountGp == size - 1
                        && isElementAmongFirst(gPRequestQueue, rank, nGunpoint) == 1
                        ){
                            pthread_cond_signal(&condition);
                        }
                    } else {
                        sendPacket(0, gpReqQueueHead->id, ACK_GP);
                        println("Wysyłam ACK_GP do procesu o miejscu w kolejce: %d", gpReqQueueHead->id);
                        nGunpoint--;
                    }
                    gpReqQueueHead = gpReqQueueHead->next;
                    count_gp++;
                }
                sort(&gPRequestQueue);
                sort(&eyeRequestQueue);

                pthread_mutex_unlock(&mutex);
                break;
            case GUN_PRODUCED:
                debug("Dostałem GUN_PRODUCED od %d, zwiekszam dostepnosc broni", status.MPI_SOURCE);
                pthread_mutex_lock(&mutex);
                nGun++;
                removeNode(&eyeRequestQueue, status.MPI_SOURCE);
                removeNode(&gPRequestQueue, status.MPI_SOURCE);
                sort(&eyeRequestQueue);
                sort(&gPRequestQueue);
                sort(&gunRequestQueue);
                struct pair_id_ts* gunReqQueueHead = gunRequestQueue;
                int count = 0;
                while (gunReqQueueHead != NULL && count < nGun) {
                    if (gunReqQueueHead->id == rank)
                    {
                        if (ackCountGun == size - 1
                        && isElementAmongFirst(gunRequestQueue, rank, nGun) == 1
                        ){
                            pthread_cond_signal(&condition);
                        }
                    } else {
                        sendPacket(0, gunReqQueueHead->id, ACK_GUN);
                        println("Wysyłam ACK_GUN do procesu o miejscu w kolejce: %d", gunReqQueueHead->id);
                        nGun--;
                    }
                    gunReqQueueHead = gunReqQueueHead->next;
                    count++;
                }
                sort(&gunRequestQueue);
                pthread_mutex_unlock(&mutex);
                break;
            default:
                break;
        }
    }
}
