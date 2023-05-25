#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
    srandom(rank);
    int tag;
    int perc;
    packet_t *pkt;
    while (stan != InFinish) {
	switch (stan) {
	    case FREE:
                perc = random()%100;
                if ( perc < 25 ) {
                    debug("Perc: %d", perc);
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
                    pkt->data = perc;
                    ackCountGp = 0;
                    ackCountEye = 0;
                    ackCountGp = 0;
                    if (strcmp(type, "GNOME") == 0) {
                        for (int i=0;i<=size-1;i++)
                            if (i!=rank) {
                                sendPacket(pkt, i, REQ_EYE);
                                sendPacket(pkt, i, REQ_GP);
                            }
                        changeState( WAITING_FOR_EYE_AND_GUNPOINT );
                        free(pkt);
                    } else {
                        for (int i=0;i<=size-1;i++)
                            if (i!=rank)
                                sendPacket( pkt, i, REQ_GUN);
                        changeState( WAITING_FOR_GUN );
                        free(pkt);
                    }
                }
                debug("Skończyłem wysyłać prośby o zasoby, teraz będę czekał na pozwolenia od innych procesów");
                break;
	    case WAITING_FOR_EYE_AND_GUNPOINT:
            println("Czekam na wejście do sekcji krytycznej, wysłałem prośby o zasoby agrafek i celowników, czekam na odpowiedzi")
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            // bo aktywne czekanie jest BUE
            if ( ackCountEye == size - 1 && ackCountGp == size - 1) {
                nEye--;
                nGunpoint--;
                changeState(PRODUCING_GUN);
            }
            break;
	    case PRODUCING_GUN:
        // tutaj zapewne jakiś muteks albo zmienna warunkowa
            println("Jestem w sekcji krytycznej, produkuję broń z zasobów");
            sleep(5);
        //if ( perc < 25 ) {
            debug("Perc: %d", perc);
            println("Wychodzę z sekcji krytycznej, wytworzyłem broń")
            debug("Zmieniam stan na wysyłanie");
            pkt = malloc(sizeof(packet_t));
            pkt->data = perc;
            nGun++;
            for (int i=0;i<=size-1;i++)
            if (i!=rank)
                sendPacket( pkt, (rank+1)%size, GUN_PRODUCED);
            changeState( FREE );
            free(pkt);
        //}
		    break;
        case WAITING_FOR_GUN:
            println("Czekam na wejście do sekcji krytycznej, wysłałem prośby o zasób broni, czekam na odpowiedzi")
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            // bo aktywne czekanie jest BUE
            if ( ackCountGun == size - 1) {
                nGun--;
                changeState(KILLING_RAT);
            }
            break;
        case KILLING_RAT:
            // tutaj zapewne jakiś muteks albo zmienna warunkowa
            println("Jestem w sekcji krytycznej, zabijam szczury");
            sleep(5);
            //if ( perc < 25 ) {
            debug("Perc: %d", perc);
            println("Wychodzę z sekcji krytycznej, zabiłem szczury")
            debug("Zmieniam stan na wysyłanie");
            pkt = malloc(sizeof(packet_t));
            pkt->data = perc;
            nEye++;
            nGunpoint++;
            for (int i=0;i<=size-1;i++)
                if (i!=rank)
                    sendPacket( pkt, (rank+1)%size, RELEASE_GUN);
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
