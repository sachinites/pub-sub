#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Libs/tlv.h"
#include "comm-types.h"
#include "cmsgOp.h"

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
