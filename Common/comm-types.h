#ifndef __COMM_TYPES__
#define __COMM_TYPES__

#include <stdint.h>

#define COORD_MSGQ_NAME     "/MAIN-COORD-MSGQ"
#define COORD_IP_ADDR   2130706433 // 127.0.0.1
#define COORD_UDP_PORT      5002

typedef enum msg_type_ {

    SUBS_TO_COORD,
    COORD_TO_SUBS, 
    PUB_TO_COORD,
    COORD_TO_PUB

}  msg_type_t;

typedef enum sub_msg_type_ {

    SUB_MSG_OK,
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
    SUB_MSG_INFORM_NEW_SUBS,
    /* confirm the generation of Pub/Sub IDs*/
    SUB_MSG_ID_ALLOC_SUCCESS,
    /* Add IPC Channel */
    SUB_MSG_IPC_CHANNEL_ADD,
    /* Remove IPC Channel */
    SUB_MSG_IPC_CHANNEL_REMOVE,
    /* ERROR */
    SUB_MSG_ERROR

} sub_msg_type_t;

typedef enum error_codes_ {

    /* cmsg_t->msg_code , TLV buffer will contain what TLVs are expected */
    ERROR_TLV_MISSING

} error_codes_t;


typedef struct cmsg_ {

    uint32_t msg_id;
    msg_type_t msg_type;
    sub_msg_type_t sub_msg_type;
    uint32_t msg_code;
    union {
        uint32_t publisher_id;
        uint32_t subscriber_id;
    } id;
    uint16_t tlv_buffer_size;
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