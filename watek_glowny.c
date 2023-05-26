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
                for (int i=0;i<=size-1;i++)
                    if (i!=rank) {
                        sendPacket(pkt, i, REQ_EYE);
                        sendPacket(pkt, i, REQ_GP);
                        ts_of_last_sent_eye_req = l_clock;
                        ts_of_last_sent_gp_req = l_clock;
                    }
                changeState( WAITING_FOR_EYE_AND_GUNPOINT );
            } else {
                for (int i=0;i<=size-1;i++)
                    if (i!=rank) {
                        sendPacket( pkt, i, REQ_GUN);
                        ts_of_last_sent_gun_req = l_clock;
                    }
                changeState( WAITING_FOR_GUN );
            }
            free(pkt);
            debug("Skończyłem wysyłać prośby o zasoby, teraz będę czekał na pozwolenia od innych procesów");
            break;
	    case WAITING_FOR_EYE_AND_GUNPOINT:
            println("Czekam na wejście do sekcji krytycznej, wysłałem prośby o zasoby agrafek i celowników, czekam na odpowiedzi")
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            // bo aktywne czekanie jest BUE
            if ( ackCountEye == size - nEye && ackCountGp == size - nGunpoint && nEyeLocal < nEye && nGunpointLocal < nGunpoint) {
                nGunpointLocal++;
                nEyeLocal++;
                changeState(PRODUCING_GUN);
            }
            break;
	    case PRODUCING_GUN:
        // tutaj zapewne jakiś muteks albo zmienna warunkowa
            println("Jestem w sekcji krytycznej, produkuję broń z zasobów");
            sleep(5);
            println("Wychodzę z sekcji krytycznej, wytworzyłem broń")
            debug("Zmieniam stan na wysyłanie");
            pkt = malloc(sizeof(packet_t));
            for (int i=0;i<=size-1;i++)
            if (i!=rank) {
                sendPacket( pkt, (rank+1)%size, GUN_PRODUCED);
            }

            currentNode = gunRequestQueue;
            while (currentNode != NULL) {
                nGunLocal++;
                sendPacket( pkt, currentNode->id, ACK_GUN);
                currentNode = currentNode->next;
            }
            nGunLocal--;
            changeState( FREE );
            free(pkt);
        //}
		    break;
        case WAITING_FOR_GUN:
            println("Czekam na wejście do sekcji krytycznej, wysłałem prośby o zasób broni, czekam na odpowiedzi")
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            // bo aktywne czekanie jest BUE
            if ( ackCountGun == size - nGun && nGunLocal < nGun) {
                nGunLocal++;
                changeState(KILLING_RAT);
            }
            break;
        case KILLING_RAT:
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            println("Jestem w sekcji krytycznej, zabijam szczury");
            sleep(5);
            println("Wychodzę z sekcji krytycznej, zabiłem szczury")
            debug("Zmieniam stan na wysyłanie");
            pkt = malloc(sizeof(packet_t));
            for (int i=0;i<=size-1;i++)
                if (i!=rank)
                    sendPacket( pkt, (rank+1)%size, RELEASE_GUN);
            nGunpointLocal--;
            nEyeLocal--;
            currentNode = gPRequestQueue;
            while (currentNode != NULL) {
                nGunpointLocal++;
                sendPacket( pkt, currentNode->id, ACK_GP);
                currentNode = currentNode->next;
            }

            currentNode = eyeRequestQueue;
            while (currentNode != NULL) {
                nEyeLocal++;
                sendPacket( pkt, currentNode->id, ACK_EYE);
                currentNode = currentNode->next;
            }
            changeState( FREE );
            free(pkt);
            //}
            break;
	    default:
		    break;
    }
        sleep(SEC_IN_STATE);
    }
}
