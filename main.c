/* w main.h także makra println oraz debug -  z kolorkami! */
#include "main.h"
#include "watek_glowny.h"
#include "watek_komunikacyjny.h"

/*
 * W main.h extern int rank (zapowiedź) w main.c int rank (definicja)
 * Zwróćcie uwagę, że każdy proces ma osobą pamięć, ale w ramach jednego
 * procesu wątki współdzielą zmienne - więc dostęp do nich powinien
 * być obwarowany muteksami. Rank i size akurat są write-once, więc nie trzeba,
 * ale zob util.c oraz util.h - zmienną state_t state i funkcję changeState
 *
 */
int rank, size;
int ackCountEye = 0;
int ackCountGun = 0;
int ackCountGp = 0;
int l_clock = 0;
sem_t l_clock_sem;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
char* type;
int nEye;
int nGunpoint;
int nBrownie;
int nGnome;
int nEyeLocal = 0;
int nGunpointLocal = 0;
int nGunLocal = 0;
int nGun = 0;
struct pair_id_ts *eyeRequestQueue;
struct pair_id_ts *gPRequestQueue;
struct pair_id_ts *gunRequestQueue;

/*
 * Każdy proces ma dwa wątki - główny i komunikacyjny
 * w plikach, odpowiednio, watek_glowny.c oraz (siurpryza) watek_komunikacyjny.c
 *
 *
 */

pthread_t threadKom;

void sort(struct pair_id_ts** head) {
    struct pair_id_ts* current;
    struct pair_id_ts* next;
    int swapped;

    if (*head == NULL) {
        return;
    }

    do {
        swapped = 0;
        current = *head;

        while (current->next != NULL) {
            next = current->next;

            if (current->ts > next->ts) {
                // Zamiana węzłów
                if (current == *head) {
                    *head = next;
                } else {
                    struct pair_id_ts* prev = *head;
                    while (prev->next != current) {
                        prev = prev->next;
                    }
                    prev->next = next;
                }
                current->next = next->next;
                next->next = current;
                swapped = 1;
            } else if (current->ts == next->ts && current->id > next->id) {
                if (current == *head) {
                    *head = next;
                } else {
                    struct pair_id_ts* prev = *head;
                    while (prev->next != current) {
                        prev = prev->next;
                    }
                    prev->next = next;
                }
                current->next = next->next;
                next->next = current;
                swapped = 1;
            }

            current = next;
        }
    } while (swapped);
}

void printList(struct pair_id_ts* head) {
    printf("RANK: %d\n", rank);
    struct pair_id_ts* current = head;

    while (current != NULL) {
        printf("ID: %d, TS: %d\n", current->id, current->ts);
        current = current->next;
    }
}

int isElementAmongFirst(struct pair_id_ts* head, int id, int x) {
    struct pair_id_ts* current = head;
    int count = 0;
    
    while (current != NULL && count < x) {
        if (current->id == id) {
            return 1;
        }

        current = current->next;
        count++;
    }

    return 0;
}

struct pair_id_ts* getElementByIndex(struct pair_id_ts* head, int index) {
    struct pair_id_ts* current = head;
    int count = 0;

    while (current != NULL) {
        if (count == index) {
            return current;
        }

        current = current->next;
        count++;
    }

    return NULL;  // Jeśli indeks wykracza poza zakres listy
}

void insert(struct pair_id_ts** head, int id, int ts) {
    struct pair_id_ts* new_node = malloc(sizeof(struct pair_id_ts));
    new_node->id = id;
    new_node->ts = ts;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        struct pair_id_ts* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void removeNode(struct pair_id_ts** head, int id) {
    if (*head == NULL) {
        return;
    }

    struct pair_id_ts* current = *head;
    struct pair_id_ts* prev = NULL;

    if (current != NULL && current->id == id) {
        *head = current->next;
        free(current);
        return;
    }

    while (current != NULL && current->id != id) {
        prev = current;
        current = current->next;
    }

    if (current == NULL) {
        return;
    }

    prev->next = current->next;
    free(current);
}


void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
    sem_destroy(&l_clock_sem);
    free(gunRequestQueue);
    free(gPRequestQueue);
    free(eyeRequestQueue);
}

// void check_thread_support(int provided)
// {
//     printf("THREAD SUPPORT: chcemy %d. Co otrzymamy?\n", provided);
//     switch (provided) {
//         case MPI_THREAD_SINGLE: 
//             printf("Brak wsparcia dla wątków, kończę\n");
//             /* Nie ma co, trzeba wychodzić */
// 	    fprintf(stderr, "Brak wystarczającego wsparcia dla wątków - wychodzę!\n");
// 	    MPI_Finalize();
// 	    exit(-1);
// 	    break;
//         case MPI_THREAD_FUNNELED: 
//             printf("tylko te wątki, ktore wykonaly mpi_init_thread mogą wykonać wołania do biblioteki mpi\n");
// 	    break;
//         case MPI_THREAD_SERIALIZED: 
//             /* Potrzebne zamki wokół wywołań biblioteki MPI */
//             printf("tylko jeden watek naraz może wykonać wołania do biblioteki MPI\n");
// 	    break;
//         case MPI_THREAD_MULTIPLE: printf("Pełne wsparcie dla wątków\n"); /* tego chcemy. Wszystkie inne powodują problemy */
// 	    break;
//         default: printf("Nikt nic nie wie\n");
//     }
// }


int main(int argc, char **argv)
{
    if (argc < 5) {
        printf("Podaj argumenty: liczba skrzatow, liczba gnomow, liczba agrawek, liczba celownikow");
        finalizuj();
    }
    nBrownie = atoi(argv[1]);
    nGnome = atoi(argv[2]);
    nEye = atoi(argv[3]);
    nGunpoint = atoi(argv[4]);
    eyeRequestQueue = NULL;
    gPRequestQueue = NULL;
    gunRequestQueue = NULL;
    sem_init(&l_clock_sem, 0, 1);
    MPI_Status status;
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    // check_thread_support(provided);
    srand(rank);
    inicjuj_typ_pakietu(); // tworzy typ pakietu
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (size != (atoi(argv[1]) + atoi(argv[2]))) {
        printf("Liczba procesow musi byc rowna liczbie gnomow plus skrzatow\n");
        printf("Podano %d procesów, %d skrzatow i %d gnomow\n", size, nBrownie, nGnome);
        finalizuj();
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank >= size - nBrownie) {
        type = "GNOME";
    } else {
        type = "BROWNIE";
    }
    // startKomWatek w watek_komunikacyjny.c
    pthread_create( &threadKom, NULL, startKomWatek , 0);

    mainLoop();
    
    finalizuj();
    return 0;
}

