#ifndef __COMM_TYPES__
#define __COMM_TYPES__

#include <stdint.h>

#define COORD_MSGQ_NAME "/MAIN-COORD-MSGQ"
#define COORD_IP_ADDR   "127.0.0.01"
#define COORD_TCP_PORT      5000
#define COORD_UDP_PORT      5002

typedef enum msg_type_ {

    SUBS_TO_COORD,
    COORD_TO_SUBS, 
    PUB_TO_COORD,
    COORD_TO_PUB

}  msg_type_t;

typedef struct cmsg_ {

    uint32_t msg_id;
    msg_type_t msg_type;
    uint32_t msg_code;
    uint32_t publisher_id;
    uint16_t msg_size;
    char msg[0];

} cmsg_t;

#endif 