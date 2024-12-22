#ifndef __COMM_TYPES__
#define __COMM_TYPES__

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "../Libs/tlv.h"

#define COORD_MSGQ_NAME     "/MAIN-COORD-MSGQ"
#define COORD_IP_ADDR   2130706433 // 127.0.0.1
#define COORD_UDP_PORT      40000

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

    SUB_MSG_DATA,
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
        case SUB_MSG_REQUEST_ACK:
            return "SUB_MSG_REQUEST_ACK";
        case SUB_MSG_ACK_MSG:
            return "SUB_MSG_ACK_MSG";
        case SUB_MSG_SUBCRIBER_LIST:
            return "SUB_MSG_SUBCRIBER_LIST";
        case SUB_MSG_INFORM_NEW_SUBS:
            return "SUB_MSG_INFORM_NEW_SUBS";
        case SUB_MSG_ID_ALLOC_SUCCESS:
            return "SUB_MSG_ID_ALLOC_SUCCESS";
        case SUB_MSG_IPC_CHANNEL_ADD:
            return "SUB_MSG_IPC_CHANNEL_ADD";
        case SUB_MSG_IPC_CHANNEL_REMOVE:
            return "SUB_MSG_IPC_CHANNEL_REMOVE";
        case SUB_MSG_ERROR:
            return "SUB_MSG_ERROR";
    }
    return "UNKNOWN";
}

typedef enum error_codes_ {

    /* cmsg_t->msg_code , TLV buffer will contain what TLVs are expected */
    ERROR_TLV_MISSING

} error_codes_t;

/* Coordinator must send an ACK for this msg back to Pub*/
#define CMSG_MSG_F_ACK_REQUIRED (1)
 /* Coordinator Must distribute this message periodically */
#define CMSG_MSG_F_PERIODIC_DIST  (2)


typedef struct cmsg_meta_data_ {

    uint8_t flags;
    uint16_t interval_sec;
    uint32_t pub_gen_id;

} cmsg_meta_data_t;

typedef enum cmsg_pr_ {

    CMSG_PR_HIGH,
    CMSG_PR_MEDIUM,
    CMSG_PR_LOW,
    CMSG_PR_MAX

} cmsg_pr_t;

typedef struct cmsg_ {

    uint32_t msg_id;
    msg_type_t msg_type;
    sub_msg_type_t sub_msg_type;
    cmsg_pr_t priority;
    uint32_t msg_code;
    union {
        uint32_t publisher_id;
        uint32_t subscriber_id;
    } id;
    cmsg_meta_data_t meta_data;
    uint32_t ref_count;
    uint16_t tlv_buffer_size;
    char msg[0];

} cmsg_t;

static inline void 
cmsg_reference (cmsg_t *cmsg) {cmsg->ref_count++;}  

static inline void 
cmsg_dereference (cmsg_t *cmsg) {
    
    assert(cmsg->ref_count);
    cmsg->ref_count--;
    if (cmsg->ref_count) return;
    free (cmsg);
}


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