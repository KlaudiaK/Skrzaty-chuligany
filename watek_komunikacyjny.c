#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while ( stan!=InFinish ) {
	debug("czekam na recv");
        MPI_Recv( &pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        sem_wait(&l_clock_sem);
        if (pakiet.ts > l_clock) {
            l_clock = pakiet.ts + 1;
        } else {
            l_clock++;
        }
        sem_post(&l_clock_sem);

        switch ( status.MPI_TAG ) {
	    case REQ_EYE:
            debug("Ktoś coś prosi. A niech ma!")
            sendPacket( 0, status.MPI_SOURCE, ACK_EYE );
	        break;
	    case REQ_GP:
            debug("Ktoś coś prosi. A niech ma!")
            sendPacket( 0, status.MPI_SOURCE, ACK_GP );
	        break;
	    case REQ_GUN:
            debug("Ktoś coś prosi. A niech ma!")
            sendPacket( 0, status.MPI_SOURCE, ACK_GUN );
	        break;
	    case ACK_EYE:
            debug("Dostałem ACK_EYE od %d, mam już %d", status.MPI_SOURCE, ackCountEye);
	        ackCountEye++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
	        break;
	    case ACK_GUN:
            debug("Dostałem ACK_GUN od %d, mam już %d", status.MPI_SOURCE, ackCountEye);
	        ackCountGun++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
	        break;
	    case ACK_GP:
            debug("Dostałem ACK_GP od %d, mam już %d", status.MPI_SOURCE, ackCountEye);
	        ackCountGp++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
	        break;
	    case RELEASE_GUN:
            debug("Dostałem RELEASE_GUN od %d, zwiekszam zasoby agrafek i celownikow", status.MPI_SOURCE);
            nEye++;
            nGunpoint++;
	        break;
	    case GUN_PRODUCED:
            debug("Dostałem GUN_PRODUCED od %d, zwiekszam dostepnosc broni", status.MPI_SOURCE);
            nGun++;
	        break;
	    default:
	        break;
        }
    }
}
