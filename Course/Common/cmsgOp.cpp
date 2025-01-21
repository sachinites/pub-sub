#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Libs/tlv.h"
#include "comm-types.h"
#include "cmsgOp.h"

cmsg_t *
cmsg_data_prepare2 (msg_type_t msg_type, 
                        sub_msg_type_t sub_msg_type, 
                        uint32_t msg_code, 
                        int trailing_space) {

    cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (cmsg_t) + trailing_space);
    msg->msg_id = 0;
    msg->msg_type = msg_type;
    msg->sub_msg_type = sub_msg_type;
    msg->msg_code = msg_code;
    msg->tlv_buffer_size = trailing_space;
    return msg;
}

void 
cmsg_debug_print (cmsg_t *cmsg) {

    printf ("Msg ID : %u | ", cmsg->msg_id);
    printf ("Msg Type : %s | ",  msg_type_to_string (cmsg->msg_type));
    printf ("Sub Msg Type : %s | ", sub_msg_type_to_string (cmsg->sub_msg_type));
    printf ("Msg Code : %u | ", cmsg->msg_code);
    printf ("Publisher ID : %u | ", cmsg->id.publisher_id);
    printf ("Subscriber ID : %u | ", cmsg->id.subscriber_id);
    printf ("TLV Buffer Size : %u | ", cmsg->tlv_buffer_size);

    char *tlv_buffer = (char *)(cmsg->tlv_buffer);
    size_t tlv_bufer_size = cmsg->tlv_buffer_size;
    uint8_t tlv_type, tlv_len;
    char *tlv_value = NULL;

    ITERATE_TLV_BEGIN(tlv_buffer, tlv_type, tlv_len, tlv_value, tlv_bufer_size) {

        printf ("TLV Type : %d | ", tlv_type);
        printf ("TLV Length : %u | ", tlv_len);
        printf ("TLV Value : %s\n", tlv_value);

    } ITERATE_TLV_END;

}
