#include <stdlib.h>
#include <stdint.h>

extern void *
pub_skt_example(void *arg);

extern uint16_t argument_port_no;

int 
main (int argc, char **argv) {

    if (argc == 2) {
        argument_port_no = atoi(argv[1]);
    }
    pub_skt_example(0);
    return 0;
}