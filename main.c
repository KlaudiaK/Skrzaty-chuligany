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
char* type;
int nEye;
int nGunpoint;
int nBrownie;
int nGnome;
int nGun = 0;
int* eyeRequestQueue;
int* gPRequestQueue;
int* gunRequestQueue;

/*
 * Każdy proces ma dwa wątki - główny i komunikacyjny
 * w plikach, odpowiednio, watek_glowny.c oraz (siurpryza) watek_komunikacyjny.c
 *
 *
 */

pthread_t threadKom;

void finalizuj()
{
    pthread_mutex_destroy( &stateMut);
    /* Czekamy, aż wątek potomny się zakończy */
    println("czekam na wątek \"komunikacyjny\"\n" );
    pthread_join(threadKom,NULL);
    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
    sem_destroy(&l_clock_sem);
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
    nEye = atoi(argv[1]);
    nGunpoint = atoi(argv[1]);
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

