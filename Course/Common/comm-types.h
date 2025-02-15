#ifndef __COMM_TYPES__
#define __COMM_TYPES__

#include <stdint.h>
#include <assert.h>
#include "../Libs/tlv.h"

#define COORD_IP_ADDR   2130706433 // 127.0.0.1
#define COORD_UDP_PORT  40000

typedef enum msg_type_ {

    SUBS_TO_COORD,
    COORD_TO_SUBS, 
    PUB_TO_COORD,
    COORD_TO_PUB

}  msg_type_t;

static inline const char *
msg_type_to_string (msg_type_t msg_type) {

    switch (msg_type) {

        case SUBS_TO_COORD:
            return "SUBS_TO_COORD";
        case COORD_TO_SUBS:
            return "COORD_TO_SUBS";
        case PUB_TO_COORD:
            return "PUB_TO_COORD";
        case COORD_TO_PUB:
            return "COORD_TO_PUB";
    }
    return "UNKNOWN";
}

typedef enum sub_msg_type_ {

    SUB_MSG_DATA, // Data msg generated by published to be conveyed to subscribers

    /* Publisher Publishing a new msg or 
    Subscriber subscribing a new msg*/
    SUB_MSG_ADD,

    /* Publisher UnPublishing a msg or 
    Subscriber Unsubscribing a msg*/
    SUB_MSG_DELETE,

    /* Pub/Sub UnRegistering it with COORD*/
    SUB_MSG_REGISTER,
    SUB_MSG_UNREGISTER,

    SUB_MSG_ID_ALLOC_SUCCESS,
    SUB_MSG_IPC_CHANNEL_ADD,

    /* ERROR */
    SUB_MSG_ERROR

} sub_msg_type_t;

static inline const char *
sub_msg_type_to_string (sub_msg_type_t sub_msg_type) {

    switch (sub_msg_type) {

        case SUB_MSG_DATA:
            return "SUB_MSG_DATA";
        case SUB_MSG_ADD:
            return "SUB_MSG_ADD";
        case SUB_MSG_DELETE:
            return "SUB_MSG_DELETE";
        case SUB_MSG_REGISTER:
            return "SUB_MSG_REGISTER";
        case SUB_MSG_UNREGISTER:
            return "SUB_MSG_UNREGISTER";
        case SUB_MSG_ID_ALLOC_SUCCESS:
            return "SUB_MSG_ID_ALLOC_SUCCESS";
        case SUB_MSG_IPC_CHANNEL_ADD:
            return "SUB_MSG_IPC_CHANNEL_ADD";
        case SUB_MSG_ERROR:
            return "SUB_MSG_ERROR";
    }
    return "UNKNOWN";
}

typedef enum cmsg_pr_ {

    CMSG_PR_HIGH,
    CMSG_PR_MEDIUM,
    CMSG_PR_LOW,
    CMSG_PR_MAX

} cmsg_pr_t;

typedef struct cmsg_ {

    /* unique ID : allocated by Coordinator only 
        if msg is generated by pub/subs , then assign msg ID as 0
    */
    uint32_t msg_id;

    /* Dirn : from Pub to Coordinator
              from Subs to Coord 
              from Coord to Pub
              from coord to Subs
    */
    msg_type_t msg_type;

    /*
        Register msg Or Unregister msg Or
        simple Data msg 
        Subscriber Msg ADD
        Subscriber Msg DEL
        etc ....
    */
    sub_msg_type_t sub_msg_type;

    cmsg_pr_t priority;
    
    uint32_t msg_code;

    union {
        uint32_t publisher_id;
        uint32_t subscriber_id;
    } id;

    uint32_t ref_count;

    uint16_t tlv_buffer_size;
    char tlv_buffer[0];

}cmsg_t;

typedef enum error_codes_ {

    /* cmsg_t->msg_code , TLV buffer will contain what TLVs are expected */
    ERROR_TLV_MISSING

} error_codes_t;

static inline void 
cmsg_reference (cmsg_t *cmsg) {cmsg->ref_count++;}  

static inline void 
cmsg_dereference (cmsg_t *cmsg) {
    
    assert(cmsg->ref_count);
    cmsg->ref_count--;
    if (cmsg->ref_count) return;
    free (cmsg);
}


/* TLV definitions */

#define TLV_CODE_NAME_LEN   32
#define TLV_IPC_NET_SKT_LEN 6 // 4B of IP Address, 2B of port number

#define TLV_CODE_NAME   1
#define TLV_IPC_TYPE_MSGQ   2   // TLV size 2 Bytes
#define TLV_IPC_TYPE_UXSKT   3 // TLV size 2 Bytes
#define TLV_IPC_TYPE_CBK     4
#define TLV_IPC_NET_UDP_SKT 5
#define TLV_DATA_128    6
#define TLV_DATA_256    7

static inline const char *
tlv_str (int tlv_code_cpoint) {

    switch (tlv_code_cpoint) {

        case TLV_CODE_NAME:
            return "TLV_CODE_NAME";
        case TLV_IPC_TYPE_MSGQ:
            return "TLV_IPC_TYPE_MSGQ";
        case TLV_IPC_TYPE_UXSKT:
            return "TLV_IPC_TYPE_UXSKT";
        case TLV_IPC_TYPE_CBK:
            return "TLV_IPC_TYPE_CBK";
        case TLV_IPC_NET_UDP_SKT:
            return "TLV_IPC_NET_UDP_SKT";
        case TLV_DATA_128:
            return "TLV_DATA_128";
        case TLV_DATA_256:
            return "TLV_DATA_256";
    }
    return "UNKNOWN";
}


static int 
tlv_data_len (int tlv_code_point) {

    switch (tlv_code_point) {

        case TLV_CODE_NAME:
            return TLV_CODE_NAME_LEN; 
        case TLV_IPC_TYPE_MSGQ:
            return 64;
        case TLV_IPC_TYPE_UXSKT:
            return 64;
        case TLV_IPC_TYPE_CBK:
            return sizeof(uintptr_t);
        case TLV_IPC_NET_UDP_SKT:
            return TLV_IPC_NET_SKT_LEN;
        case TLV_DATA_128:
            return 128;
        case TLV_DATA_256:
            return 256;
    }
    return 0;
}


#endif