#include "../Common/ipc_struct.h"

extern void *
sub_example (void *ipc_struct) ;

int 
main (int argc, char **argv) {

    ipc_struct_t ipc_struct;
    ipc_struct.netskt.ip_addr =   2130706433 ; //127.0.0.1
    ipc_struct.netskt.port = 50001;

    sub_example ((void *)&ipc_struct);

    return 0;
}