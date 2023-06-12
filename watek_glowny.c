#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    struct pair_id_ts* currentNode;
    srandom(rank);
    packet_t *pkt;
    while (stan != InFinish) {
	switch (stan) {
        case FREE:
            sleep(1 + rand() % 5);
            if (strcmp(type, "GNOME") == 0) {
                println("JESTEM GNOMEM");
                println("Ubiegam się o zasoby Agrafka i Celownik, chce wejsc do sekcji krytycznej i stworzyć broń");
            }
            else {
                println("JESTEM SKRZATEM");
                println("Ubiegam się o zasoby Broni, chce wejsc do sekcji krytycznej i zabijać szczury");
            }

            debug("Zmieniam stan na wysyłanie");
            pkt = malloc(sizeof(packet_t));
            ackCountGp = 0;
            ackCountEye = 0;
            ackCountGp = 0;
            if (strcmp(type, "GNOME") == 0) {
                insert(&eyeRequestQueue, rank, l_clock);
                insert(&gPRequestQueue, rank, l_clock);
                sort(&eyeRequestQueue);
                sort(&gPRequestQueue);
                for (int i=0;i<=size-1;i++){
                        println("%d", i);
                        sendPacketWithoutIncreasingTimeStamp(pkt, i, REQ_EYE);
                        sendPacketWithoutIncreasingTimeStamp(pkt, i, REQ_GP);
                }
                sem_wait(&l_clock_sem);
                ts_of_last_sent_eye_req = l_clock;
                ts_of_last_sent_gp_req = l_clock;
                l_clock++;
                sem_post(&l_clock_sem);
                changeState( WAITING_FOR_EYE_AND_GUNPOINT );

            } else {
                insert(&gunRequestQueue, rank, l_clock);
                sort(&gunRequestQueue);
                for (int i=0;i<=size-1;i++)
                    if (i!=rank) {
                        sendPacketWithoutIncreasingTimeStamp( pkt, i, REQ_GUN);
                    }

                sem_wait(&l_clock_sem);
                ts_of_last_sent_gun_req = l_clock;
                l_clock++;
                sem_post(&l_clock_sem);
                changeState( WAITING_FOR_GUN );
            }
            free(pkt);
            debug("Skończyłem wysyłać prośby o zasoby, teraz będę czekał na pozwolenia od innych procesów");
            break;
	    case WAITING_FOR_EYE_AND_GUNPOINT:
            println("Czekam na wejście do sekcji krytycznej, wysłałem prośby o zasoby agrafek i celowników, czekam na odpowiedzi")
            println("MAM %d ack na eye i %d ack na gp", ackCountEye, ackCountGp);
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            // bo aktywne czekanie jest BUE
            pthread_mutex_lock(&mutex);
            while (ackCountEye < size - 1 || ackCountGp < size - 1
            || isElementAmongFirst(eyeRequestQueue, rank, nEye) != 1
            || isElementAmongFirst(gPRequestQueue, rank, nGunpoint) != 1
            ) {
                pthread_cond_wait(&condition, &mutex);
            }
            pthread_mutex_unlock(&mutex);

            sem_wait(&l_clock_sem);
            nEye--;
            nGunpoint--;
            sem_post(&l_clock_sem);
            changeState(PRODUCING_GUN);
            break;
	    case PRODUCING_GUN:
        // tutaj zapewne jakiś muteks albo zmienna warunkowa
            println("Jestem w sekcji krytycznej, produkuję broń z zasobów");
            sleep(1 + rand() % 5);
            println("Wychodzę z sekcji krytycznej, wytworzyłem broń")
            debug("Zmieniam stan na wysyłanie");
            pkt = malloc(sizeof(packet_t));
            for (int i=0;i<=size-1;i++)
            if (i!=rank) {
                sendPacket( pkt, i, GUN_PRODUCED);
            }
            sem_wait(&l_clock_sem);
            nGun++;
           
            removeNode(&eyeRequestQueue, rank);
            
            removeNode(&gPRequestQueue, rank);
            sort(&eyeRequestQueue);
            sort(&gPRequestQueue);
            struct pair_id_ts* gunReqQueueHead = gunRequestQueue;
            printf("Gun requests and guns: %d \n", nGun);
                
            printList(gunReqQueueHead);
            int count = 0;
            debug("Jestem przed wyslaniem Ack_GUN ");
            sem_post(&l_clock_sem);
            while (gunReqQueueHead != NULL && count < nGun) {
                debug("Chce wyslac Ack_GUN do %d", gunReqQueueHead->id);
                sendPacket( 0, gunReqQueueHead->id, ACK_GUN );
            
                gunReqQueueHead = gunReqQueueHead->next;
                count++;
                nGun--;
            }
            sort(&gunRequestQueue);
            
            changeState( FREE );
            free(pkt);
        //}
		    break;
        case WAITING_FOR_GUN:
            println("Czekam na wejście do sekcji krytycznej, wysłałem prośby o zasób broni, czekam na odpowiedzi")
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            // bo aktywne czekanie jest BUE
            println("Czeka naaaaaaa wejście z ackCountGun %d, jest wsrod pierwszych %d", ackCountGun, isElementAmongFirst(gunRequestQueue, rank, nGun));
            while (ackCountGun <  size - 1 ||
            isElementAmongFirst(gunRequestQueue, rank, nGun) != 1) {
                pthread_cond_wait(&condition, &mutex);
            }
            pthread_mutex_unlock(&mutex);

            sem_wait(&l_clock_sem);
            nGun--;
            sem_post(&l_clock_sem);
            changeState(KILLING_RAT);
            break;
        case KILLING_RAT:
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            println("Jestem w sekcji krytycznej, zabijam szczury");
            sleep(1 + rand() % 5);
            println("Wychodzę z sekcji krytycznej, zabiłem szczury")
            debug("Zmieniam stan na wysyłanie");
            pkt = malloc(sizeof(packet_t));
            for (int i=0;i<=size-1;i++)
                if (i!=rank)
                    sendPacket( pkt, i, RELEASE_GUN);
            sem_wait(&l_clock_sem);
            nEye++;
            nGunpoint++;
            removeNode(&gunRequestQueue, rank);
            sort(&gunRequestQueue);
            struct pair_id_ts* eyeReqQueueHead = eyeRequestQueue;
            int count_eye = 0;
             sem_post(&l_clock_sem);
             pthread_mutex_lock(&mutex);
          ////   println("Eye request list");
          //   printList(eyeReqQueueHead);
            while (eyeReqQueueHead != NULL && count_eye < nEye) {
                sendPacket( 0, eyeReqQueueHead->id, ACK_EYE );
                  removeNode(&eyeRequestQueue, eyeReqQueueHead->id);
                eyeReqQueueHead = eyeReqQueueHead->next;
               
                count_eye++;
                nEye--;
            }
            struct pair_id_ts* gpReqQueueHead = gPRequestQueue;
            int count_gp = 0;
        //     println("Gunpoint request list");
         //    printList(gpReqQueueHead);
            while (gpReqQueueHead != NULL && count_gp < nGunpoint) {
                sendPacket( 0, gpReqQueueHead->id, ACK_GP );
               
                removeNode(&gPRequestQueue, gpReqQueueHead->id);
                 gpReqQueueHead = gpReqQueueHead->next;
                count_gp++;
                nGunpoint--;
            }
            sort(&gPRequestQueue);
            sort(&gPRequestQueue);
           pthread_mutex_unlock(&mutex);
            changeState( FREE );
            free(pkt);
            //}
            break;
	    default:
		    break;
    }
        sleep(SEC_IN_STATE);
        sleep(1 + rand() % 5);
    }
}
