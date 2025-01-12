#include "../Common/ipc_struct.h"

extern void *
pub_skt_example (void *ipc_struct) ;

int 
main (int argc, char **argv) {

    ipc_struct_t ipc_struct;
    ipc_struct.netskt.ip_addr =   2130706433 ; //127.0.0.1
    ipc_struct.netskt.port = 50000;

    pub_skt_example ((void *)&ipc_struct);

    return 0;
}