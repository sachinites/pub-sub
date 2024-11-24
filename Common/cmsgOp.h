#ifndef __CMSG_OP__
#define __CMSG_OP__


cmsg_t *
cord_prepare_msg (msg_type_t msg_type, 
                        sub_msg_type_t sub_msg_type, 
                        uint32_t msg_code, 
                        bool alloc_tlv_value_buffers,
                        int tlv_count, ...) ;


void 
cmsg_debug_print (cmsg_t *cmsg);

#endif