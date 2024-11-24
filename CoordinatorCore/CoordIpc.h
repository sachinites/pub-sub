#ifndef __COORD_IPC_H__
#define __COORD_IPC_H__

#include <stdint.h>

typedef enum ipc_type_ {

    IPC_TYPE_NONE,
    IPC_TYPE_NETSKT,
    IPC_TYPE_MSGQ,
    IPC_TYPE_UXSKT,
    IPC_TYPE_SHM,
    IPC_TYPE_CBK 

} ipc_type_t;

typedef union ipc_struct_ {

    struct {

        uint32_t ip_addr;
        uint16_t port;
        uint8_t transport_type;
        /* Dynamically computed */
        int sock_fd;

    } netskt;

    struct {

        char MsgQName[64];

    } msgq;

    struct {

        uint32_t UnixSktName[64];

    } uxskt;

    
}  ipc_struct_t;


#endif 
