#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr) {
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;

    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (stan != InFinish) {
        printList(eyeRequestQueue);debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        sem_wait(&l_clock_sem);
        if (pakiet.ts > l_clock) {
            l_clock = pakiet.ts + 1;
        } else {
            l_clock++;
        }
        sem_post(&l_clock_sem);

        if (strcmp(type, "GNOME") == 0) {
            switch (status.MPI_TAG) {
                case REQ_EYE:
                    println("Dostałem EYE_REQ od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack",
                            status.MPI_SOURCE, nEye);
                    if (stan == WAITING_FOR_EYE_AND_GUNPOINT) {
                        println("Ubiegam sie o sekcje krytyczna, sprawdze timestamp procesu ubiegajacego sie o zasob");
                        if (pakiet.ts < ts_of_last_sent_eye_req) {
                            println("Proces %d ma niższy timestamp od mojego, wysyłam mu ACK", status.MPI_SOURCE)
                            nEyeLocal++;
                            sendPacket(0, status.MPI_SOURCE, ACK_EYE);
                        } else if (pakiet.ts == ts_of_last_sent_eye_req) {
                            if (status.MPI_SOURCE < rank) {
                                println("Proces %d ma równy timestamp, ale niższy rank, wysyłam mu ACK",
                                        status.MPI_SOURCE);
                                nEyeLocal++;
                                sendPacket(0, status.MPI_SOURCE, ACK_EYE);
                            } else {
                                println("Proces %d ma równy timestamp, ale wyższy rank, NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                        status.MPI_SOURCE);
                                insert(&eyeRequestQueue, status.MPI_SOURCE, pakiet.ts);
                            }
                        } else {
                            println("Proces %d ma wyższy timestamp, NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                    status.MPI_SOURCE);
                            insert(&eyeRequestQueue, status.MPI_SOURCE, pakiet.ts);
                        }
                    } else if (stan == PRODUCING_GUN) {
                        println("Jestem w sekcji krytycznej NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                status.MPI_SOURCE);
                        insert(&eyeRequestQueue, status.MPI_SOURCE, pakiet.ts);
                    } else {
                        println("Wysyłam ack bez sprawdzania");
                        nEyeLocal++;
                        sendPacket(0, status.MPI_SOURCE, ACK_EYE);
                    }
                    break;
                case REQ_GP:
                    println("Dostałem GP_REQ od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack",
                            status.MPI_SOURCE, nGunpoint);
                    if (stan == WAITING_FOR_EYE_AND_GUNPOINT) {
                        println("Ubiegam sie o sekcje krytyczna, sprawdze timestamp procesu ubiegajacego sie o zasob");
                        if (pakiet.ts < ts_of_last_sent_gp_req) {
                            println("Proces %d ma niższy timestamp od mojego, wysyłam mu ACK", status.MPI_SOURCE);
                            nGunpointLocal++;
                            sendPacket(0, status.MPI_SOURCE, ACK_GP);
                        } else if (pakiet.ts == ts_of_last_sent_gp_req) {
                            if (status.MPI_SOURCE < rank) {
                                println("Proces %d ma równy timestamp, ale niższy rank, wysyłam mu ACK",
                                        status.MPI_SOURCE);
                                nGunpointLocal++;
                                sendPacket(0, status.MPI_SOURCE, ACK_GP);
                            } else {
                                println("Proces %d ma równy timestamp, ale wyższy rank, NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                        status.MPI_SOURCE);
                                insert(&gPRequestQueue, status.MPI_SOURCE, pakiet.ts);
                            }
                        } else {
                            println("Proces %d ma wyższy timestamp, NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                    status.MPI_SOURCE);
                            insert(&gPRequestQueue, status.MPI_SOURCE, pakiet.ts);
                        }
                    } else if (stan == PRODUCING_GUN) {
                        println("Jestem w sekcji krytycznej NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                status.MPI_SOURCE);
                        insert(&gPRequestQueue, status.MPI_SOURCE, pakiet.ts);
                    } else {
                        println("Wysyłam ack bez sprawdzania");
                        nGunpointLocal++;
                        sendPacket(0, status.MPI_SOURCE, ACK_GP);
                        break;
                        case REQ_GUN:
                            println("Dostałem GUN_REQ od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack",
                                    status.MPI_SOURCE, nGun);
                        println("Jestem gnomem i nie interesuje mnie ten zasób, wysyłam ACK");
                        nGunLocal++;
                        sendPacket(0, status.MPI_SOURCE, ACK_GUN);
                        break;
                        case RELEASE_GUN:
                            nEyeLocal--;
                        nGunpointLocal--;
                        break;
                        case GUN_PRODUCED:
                            nGunLocal--;
                        break;
                        case ACK_GP:
                            println("Dostałem ACK_GP od procesu %d, mam już %d", status.MPI_SOURCE, ackCountGp);
                        ackCountGp++;
                        break;
                        case ACK_EYE:
                            println("Dostałem ACK_EYE od procesu %d, mam już %d", status.MPI_SOURCE, ackCountEye);
                        ackCountEye++;
                        break;
                    }
                }
            } else if (strcmp(type, "BROWNIE") == 0) {
                switch (status.MPI_TAG) {
                    case REQ_EYE:
                        println("Dostałem REQ_EYE od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack",
                                status.MPI_SOURCE, nEye);
                        println("Jestem SKRZATEM i nie interesuje mnie ten zasób, wysyłam ACK");
                        nEyeLocal++;
                        sendPacket(0, status.MPI_SOURCE, ACK_EYE);
                        break;
                    case REQ_GP:
                        println("Dostałem ACK_GP od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack",
                                status.MPI_SOURCE, nGunpoint);
                        println("Jestem gnomem i nie interesuje mnie ten zasób, wysyłam ACK");
                        nGunpointLocal++;
                        sendPacket(0, status.MPI_SOURCE, ACK_GP);
                        break;
                    case REQ_GUN:
                        println("Dostałem GUN_REQ od procesu %d, mam %d tego zasobu, sprawdzam czy moge wysłać ack",
                                status.MPI_SOURCE, nGun);
                        if (stan == WAITING_FOR_GUN) {
                            println("Ubiegam sie o sekcje krytyczna, sprawdze timestamp procesu ubiegajacego sie o zasob");
                            if (pakiet.ts < ts_of_last_sent_gun_req) {
                                println("Proces %d ma niższy timestamp od mojego, wysyłam mu ACK", status.MPI_SOURCE);
                                nGunLocal++;
                                sendPacket(0, status.MPI_SOURCE, ACK_GUN);
                            } else if (pakiet.ts == ts_of_last_sent_gun_req) {
                                if (status.MPI_SOURCE < rank) {
                                    println("Proces %d ma równy timestamp, ale niższy rank, wysyłam mu ACK",
                                            status.MPI_SOURCE);
                                    nGunLocal++;
                                    sendPacket(0, status.MPI_SOURCE, ACK_GUN);
                                } else {
                                    println("Proces %d ma równy timestamp, ale wyższy rank, NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                            status.MPI_SOURCE);
                                    insert(&gunRequestQueue, status.MPI_SOURCE, pakiet.ts);
                                }
                            } else {
                                println("Proces %d ma wyższy timestamp, NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                        status.MPI_SOURCE);
                                insert(&gunRequestQueue, status.MPI_SOURCE, pakiet.ts);
                            }
                        } else if (stan == KILLING_RAT) {
                            println("Jestem w sekcji krytycznej NIE wysyłam mu ACK, dodaje na liste oczekujacych",
                                    status.MPI_SOURCE);
                            insert(&gunRequestQueue, status.MPI_SOURCE, pakiet.ts);
                        } else {
                            println("Wysyłam ack bez sprawdzania");
                            nGunLocal++;
                            sendPacket(0, status.MPI_SOURCE, ACK_GUN);
                        }
                        break;
                    case RELEASE_GUN:
                        nEyeLocal--;
                        nGunpointLocal--;
                        break;
                    case GUN_PRODUCED:
                        nGunLocal--;
                        break;
                    case ACK_GUN:
                        println("Dostałem ACK_GUN od procesu %d, mam już %d", status.MPI_SOURCE, ackCountGun);
                        ackCountGun++;
                        break;
            }
        }
    }
}
