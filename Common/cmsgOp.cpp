#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "../Libs/tlv.h"
#include "comm-types.h"
#include "cmsgOp.h"

cmsg_t *
cmsg_data_prepare (msg_type_t msg_type, 
                        sub_msg_type_t sub_msg_type, 
                        uint32_t msg_code, 
                        bool alloc_tlv_value_buffers,
                        int tlv_count, ...) {

    int i, tlv_code;
    va_list tlv_ids_list;

    uint16_t tlv_buffer_size = (TLV_OVERHEAD_SIZE * tlv_count);

    if (alloc_tlv_value_buffers) {

        va_start (tlv_ids_list, tlv_count);

        for ( i = 0; i < tlv_count; i++) {
            tlv_code = va_arg (tlv_ids_list, int);
            tlv_buffer_size += tlv_data_len (tlv_code);    
        }

        va_end (tlv_ids_list);
    }

    cmsg_t *reply_msg = (cmsg_t *)calloc (1, sizeof (*reply_msg) + tlv_buffer_size);
    reply_msg->msg_id = 0;
    reply_msg->msg_type = msg_type;
    reply_msg->sub_msg_type = sub_msg_type;
    reply_msg->msg_code = msg_code;
    reply_msg->tlv_buffer_size = tlv_buffer_size;

    char *tlv_buffer = (char *)reply_msg->msg;

    va_start (tlv_ids_list, tlv_count);

    for (i = 0; i < tlv_count; i++) {

        tlv_code = va_arg (tlv_ids_list, int);
        tlv_buffer_insert_tlv (tlv_buffer, tlv_code, 
            alloc_tlv_value_buffers ? tlv_data_len (tlv_code) : 0, 
            NULL);
    }

    va_end (tlv_ids_list);
    return reply_msg;
}

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

    char *tlv_buffer = (char *)(cmsg->msg);
    size_t tlv_bufer_size = cmsg->tlv_buffer_size;
    uint8_t tlv_type, tlv_len;
    char *tlv_value = NULL;

    ITERATE_TLV_BEGIN(tlv_buffer, tlv_type, tlv_len, tlv_value, tlv_bufer_size) {

        printf ("TLV Type : %s | ", tlv_str (tlv_type));
        printf ("TLV Length : %u | ", tlv_len);
        printf ("TLV Value : %s\n", tlv_value);

    } ITERATE_TLV_END;

}
