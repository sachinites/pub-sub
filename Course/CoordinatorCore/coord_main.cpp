#include <pthread.h>

extern void
coordinator_main ();

int 
main (int argc, char **argv) {

    coordinator_main ();
    pthread_exit(0);
    return 0;
}