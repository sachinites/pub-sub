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

typedef enum sub_msg_type_ {

    /* Publisher Publishing a new msg or 
        Subscriber subscribing a new msg*/
    SUB_MSG_ADD,
    /* Publisher UnPublishing a new msg or 
        Subscriber Unsubscribing a new msg*/
    SUB_MSG_DELETE,
    /* Pub/Sub Registering it with COORD*/
    SUB_MSG_REGISTER,
    /* Pub/Sub UnRegistering it with COORD*/
    SUB_MSG_UNREGISTER,
    /* Publisher Requesting ACK for the message sent */
    SUB_MSG_REQUEST_ACK,
    /* Subscriber sending back ACK*/
    SUB_MSG_ACK_MSG,
    /* Publisher request COORD to tell him the 
    subscriber List*/
    SUB_MSG_SUBCRIBER_LIST,
    /* Publisher requesting COORD to tell him when
    new Subsc Join*/
    SUB_MSG_INFORM_NEW_SUBS
} sub_msg_type_t;

typedef struct cmsg_ {

    uint32_t msg_id;
    msg_type_t msg_type;
    sub_msg_type_t sub_msg_type;
    uint32_t msg_code;
    uint32_t publisher_id;
    uint16_t msg_size;
    char msg[0];

} cmsg_t;

#define TLV_CODE_NAME   1
#define TLV_CODE_NAME_LEN   32

#define TLV_IPC_TYPE_MSGQ   2   // TLV size 2 Bytes
#define TLV_IPC_TYPE_UXSKT   3 // TLV size 2 Bytes

#define TLV_IPC_NET_SKT 4
#define TLV_IPC_NET_SKT_LEN 6 // 4B of IP Address, 2B of port number

#endif 