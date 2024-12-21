#ifndef __PIB_SUB_CLIENT_H__
#define __PIB_SUB_CLIENT_H__

#include "../Common/comm-types.h"
#include "../Common/cmsgOp.h"
#include "../Common/ipc_struct.h"

void
coordinator_register (int sock_fd, 
                                    char *entity_name, 
                                    msg_type_t msg_type);

void
coordinator_unregister (int sock_fd, 
                                       uint32_t pub_sub_id, 
                                       msg_type_t msg_type) ;

void 
publisher_publish (int sock_fd, uint32_t pub_id, uint32_t msg_code);

void 
publisher_unpublish (int sock_fd, uint32_t pub_id, uint32_t msg_code);;

 void 
subscriber_subscribe (int sock_fd, uint32_t sub_id, uint32_t msg_id);

void 
subscriber_unsubscribe (int sock_fd, uint32_t sub_id, uint32_t msg_id);

void 
subscriber_subscribe_ipc_channel (int sock_fd, 
                                uint32_t sub_id, 
                                ipc_type_t ipc_type, 
                                ipc_struct_t *ipc_struct);

int 
pub_sub_dispatch_cmsg (
                                int sock_fd, 
                               cmsg_t *cmsg);

#endif // __PIB-SUB_CLIENT_H__