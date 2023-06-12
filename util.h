#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct {
    int ts;       /* timestamp (zegar lamporta */
    int src;  

    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;

struct pair_id_ts {
    int id;
    int ts;
    struct pair_id_ts* next;
};

#define NITEMS 3 // wielkość sekcji krytycznej
/* Typy wiadomości */
/* TYPY PAKIETÓW */
#define ACK_EYE 1
#define REQ_EYE 2
#define ACK_GP 3
#define REQ_GP 4
#define ACK_GUN 5
#define REQ_GUN 6
#define RELEASE 7
#define APP_PKT 8
#define RELEASE_GUN 10
#define GUN_PRODUCED 11
#define FINISH  12

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);
void sendPacketWithoutIncreasingTimeStamp(packet_t *pkt, int destination, int tag);

typedef enum {
    FREE,
    WAITING_FOR_EYE_AND_GUNPOINT,
    PRODUCING_GUN,
    WAITING_FOR_GUN,
    KILLING_RAT,
    InFinish
} state_t;

extern state_t stan;
extern pthread_mutex_t stateMut;
extern int ts_of_last_sent_eye_req;
extern int ts_of_last_sent_gp_req;
extern int ts_of_last_sent_gun_req;
/* zmiana stanu, obwarowana muteksem */
void changeState( state_t );
#endif
