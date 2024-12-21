#ifndef __CMSG_OP__
#define __CMSG_OP__


cmsg_t *
cmsg_data_prepare (msg_type_t msg_type, 
                        sub_msg_type_t sub_msg_type, 
                        uint32_t msg_code, 
                        bool alloc_tlv_value_buffers,
                        int tlv_count, ...) ;

cmsg_t *
cmsg_data_prepare2 (msg_type_t msg_type, 
                        sub_msg_type_t sub_msg_type, 
                        uint32_t msg_code, 
                        int trailing_space) ;
                        
void 
cmsg_debug_print (cmsg_t *cmsg);

#endif