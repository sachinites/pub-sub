#include <stdlib.h>
#include <stdint.h>

extern void *
sub_skt_example(void *arg);

extern uint16_t argument_port_no;

int 
main (int argc, char **argv) {

    if (argc == 2) {
        argument_port_no = atoi(argv[1]);
    }
    sub_skt_example(0);
    return 0;
}