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
            ackCountGun = 0;
            if (strcmp(type, "GNOME") == 0) {
                insert(&eyeRequestQueue, rank, l_clock);
                insert(&gPRequestQueue, rank, l_clock);
                sort(&eyeRequestQueue);
                sort(&gPRequestQueue);
                for (int i=0;i<=size-1;i++)
                    if (i!=rank) {
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
            println("Dostepnosc zasobow to eye: %d gp: %d", nEye, nGunpoint);
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            // bo aktywne czekanie jest BUE
            pthread_mutex_lock(&mutex);
            while (!(ackCountEye == size - 1 || ackCountGp == size - 1
            && isElementAmongFirst(eyeRequestQueue, rank, nEye) == 1
            && isElementAmongFirst(gPRequestQueue, rank, nGunpoint) == 1
            )) {
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
            int count = 0;
            println("Lista gunReqQueHead\n");
            printList(gunReqQueueHead);
             sem_post(&l_clock_sem);
            while (gunReqQueueHead != NULL && count < nGun) {
                println("Wszedlem do petli gunReqQuee\n");
                int procId = gunReqQueueHead->id;
                sendPacket( 0, procId, ACK_GUN );
                gunReqQueueHead = gunReqQueueHead->next;
                removeNode(&gunRequestQueue, procId);
                count++;
            }
            gunRequestQueue = gunReqQueueHead;
            sort(&gunRequestQueue);
           
            changeState( FREE );
            free(pkt);
        //}
		    break;
        case WAITING_FOR_GUN:
            println("Czekam na wejście do sekcji krytycznej, wysłałem prośby o zasób broni, czekam na odpowiedzi")
            println("MAM %d ack na gun", ackCountGun);
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            // bo aktywne czekanie jest BUE
            while (!(ackCountGun == size - 1 &&
            isElementAmongFirst(gunRequestQueue, rank, nGun) == 1 )) {
                println("DZaczynam czekanie w petli while\n");
                pthread_cond_wait(&condition, &mutex);
            }
            pthread_cond_signal(&condition);
            pthread_mutex_unlock(&mutex);
            println("Dostalem sie dalej\n");
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
            while (eyeReqQueueHead != NULL && count_eye < nEye) {
                int procId = eyeReqQueueHead->id;
                sendPacket( 0, procId, ACK_EYE );
                eyeReqQueueHead = eyeReqQueueHead->next;
                removeNode(&eyeRequestQueue, procId);
                count_eye++;
            }
            eyeRequestQueue = eyeReqQueueHead;
            struct pair_id_ts* gpReqQueueHead = gPRequestQueue;
            int count_gp = 0;
            while (gpReqQueueHead != NULL && count_gp < nGunpoint) {
                int procId = gpReqQueueHead->id;
                sendPacket( 0, procId, ACK_GP );
                gpReqQueueHead = gpReqQueueHead->next;
                removeNode(&gpReqQueueHead, procId);
                count_gp++;
            }
            gPRequestQueue = gpReqQueueHead;
            sort(&eyeRequestQueue);
            sort(&gPRequestQueue);
            
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
