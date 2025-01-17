#include <pthread.h>

extern void 
coordinator_main();

int 
main (int argc, char **argv) {

    /* We are launching a separate thread which implements 
        Coord functionality. This is done because we will have flexibility
        to run corrd as a sub-thread of another process. If this is required
        Then we simply need to omit this file while creating executable
        of that process and coordinator will be launched as a supporting thread
        in the context of that process */
    coordinator_main ();
    pthread_exit(0);
    return 0;
}