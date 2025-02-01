#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "../Libs/tlv.h"
#include "client.h"

static int 
ipc_type_to_tlv_type (ipc_type_t ipc_type) {

    switch (ipc_type) {
        case IPC_TYPE_MSGQ:
            return TLV_IPC_TYPE_MSGQ;
        case IPC_TYPE_UXSKT:
            return TLV_IPC_TYPE_UXSKT;
        case IPC_TYPE_NETSKT:
            return TLV_IPC_NET_UDP_SKT;
        case IPC_TYPE_CBK:
            return TLV_IPC_TYPE_CBK;
        default:
            return 0;
    }
}

int 
pub_sub_dispatch_cmsg (
                       int sock_fd, 
                       cmsg_t *cmsg) {

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(COORD_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(COORD_IP_ADDR);
    int rc = sendto (sock_fd, (char *)cmsg, sizeof (*cmsg) + cmsg->tlv_buffer_size, 0, 
        (struct sockaddr *)&server_addr, sizeof (struct sockaddr));
    return rc;          

}

void 
coordinator_register (int sock_fd, 
                       const char *entity_name, 
                       msg_type_t msg_type) {
    
    cmsg_t *cmsg = (cmsg_t *)calloc 
        (1, sizeof (*cmsg) + TLV_OVERHEAD_SIZE + TLV_CODE_NAME_LEN);

    cmsg->msg_id = 0;
    cmsg->msg_type = msg_type;
    cmsg->sub_msg_type = SUB_MSG_REGISTER;
    cmsg->id.publisher_id = 0;
    cmsg->id.subscriber_id = 0;
    cmsg->tlv_buffer_size = TLV_OVERHEAD_SIZE + TLV_CODE_NAME_LEN;
    char *tlv_buffer = (char *)cmsg->tlv_buffer;
    tlv_buffer_insert_tlv (tlv_buffer, 
        TLV_CODE_NAME, TLV_CODE_NAME_LEN, (char *)entity_name);
    
    int rc = pub_sub_dispatch_cmsg(sock_fd, cmsg);
    if (rc < 0) {
        printf ("Client : Error : Send Failed, errno = %d\n", errno);
    }
    cmsg_debug_print (cmsg);
    free(cmsg);
 }

void 
coordinator_unregister (int sock_fd, 
                       uint32_t pub_sub_id, 
                       msg_type_t msg_type) {
    

    cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (*msg) );

    msg->msg_id =0;     // This will be assigned by coordinator
    msg->msg_type = msg_type;
    msg->sub_msg_type = SUB_MSG_UNREGISTER;
    msg->id.publisher_id = pub_sub_id;
    msg->id.subscriber_id = pub_sub_id;
    msg->tlv_buffer_size = 0;
    
    int rc = pub_sub_dispatch_cmsg (sock_fd, msg);
    if (rc < 0) {
        printf ("Client : Error : Send Failed, errno = %d\n", errno);
    }

    //cmsg_debug_print (msg);
    free(msg);    
 }


void 
publisher_publish (int sock_fd, uint32_t pub_id, uint32_t msg_code){

    cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (*msg));
    msg->msg_id =0; // This will be assigned by coordinator
    msg->msg_type = PUB_TO_COORD;
    msg->sub_msg_type = SUB_MSG_ADD;
    msg->msg_code = msg_code;
    msg->id.publisher_id = pub_id; // This will be assigned by coordinator
    msg->id.subscriber_id = pub_id;
    msg->tlv_buffer_size = 0;

    int rc = pub_sub_dispatch_cmsg (sock_fd, msg);
    if (rc < 0) {
        printf ("Client : Error : Send Failed, errno = %d\n", errno);
    }

    //cmsg_debug_print (msg);
    free(msg);    
}

void 
publisher_unpublish (int sock_fd, uint32_t pub_id, uint32_t msg_code) {

        cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (*msg));
    msg->msg_id =0; // This will be assigned by coordinator
    msg->msg_type = PUB_TO_COORD;
    msg->sub_msg_type = SUB_MSG_DELETE;
    msg->msg_code = msg_code;
    msg->id.publisher_id = pub_id; // This will be assigned by coordinator
    msg->id.subscriber_id = pub_id;
    msg->tlv_buffer_size = 0;

    int rc = pub_sub_dispatch_cmsg (sock_fd, msg);
    if (rc < 0) {
        printf ("Client : Error : Send Failed, errno = %d\n", errno);
    }

    //cmsg_debug_print (msg);
    free(msg); 
}

 void 
subscriber_subscribe (int sock_fd, uint32_t sub_id, uint32_t msg_id) {

    cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (*msg));
    msg->msg_id =0; // This will be assigned by coordinator
    msg->msg_type = SUBS_TO_COORD;
    msg->sub_msg_type = SUB_MSG_ADD;
    msg->msg_code = msg_id;
    msg->id.publisher_id = sub_id; // This will be assigned by coordinator
    msg->id.subscriber_id = sub_id;
    msg->tlv_buffer_size = 0;

    int rc = pub_sub_dispatch_cmsg (sock_fd, msg);
    if (rc < 0) {
        printf ("Client : Error : Send Failed, errno = %d\n", errno);
    }

    //cmsg_debug_print (msg);
    free(msg);
}

 void 
subscriber_unsubscribe (int sock_fd, uint32_t sub_id, uint32_t msg_id) {

    cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (*msg));
    msg->msg_id =0; // This will be assigned by coordinator
    msg->msg_type = SUBS_TO_COORD;
    msg->sub_msg_type = SUB_MSG_DELETE;
    msg->msg_code = msg_id;
    msg->id.publisher_id = sub_id; // This will be assigned by coordinator
    msg->id.subscriber_id = sub_id;
    msg->tlv_buffer_size = 0;
    int rc = pub_sub_dispatch_cmsg (sock_fd, msg);
    if (rc < 0) {
        printf ("Client : Error : Send Failed, errno = %d\n", errno);
    }
    //cmsg_debug_print (msg);
    free(msg);
}

void 
subscriber_subscribe_ipc_channel (
                                int sock_fd, 
                                uint32_t sub_id, 
                                ipc_type_t ipc_type, 
                                ipc_struct_t *ipc_struct) {

    
    int ipc_tlv = ipc_type_to_tlv_type (ipc_type);
    
    if (!ipc_tlv) {
        printf ("Client : Error : Invalid IPC Type\n");
        return;
    }

    uint8_t tlv_size = tlv_data_len (ipc_tlv);

    cmsg_t *subscriber_ipc_msg = cmsg_data_prepare2 (
                                                            SUBS_TO_COORD, 
                                                            SUB_MSG_IPC_CHANNEL_ADD,
                                                            0,  TLV_OVERHEAD_SIZE + tlv_size);

    subscriber_ipc_msg->id.subscriber_id = sub_id;
    subscriber_ipc_msg->msg_id = 0; // This will be assigned by coordinator

    char *tlv_buffer = (char *)subscriber_ipc_msg->tlv_buffer;
    uint8_t tlv_buffer_len = subscriber_ipc_msg->tlv_buffer_size;

    tlv_buffer_insert_tlv (tlv_buffer, ipc_tlv, tlv_size, NULL);

    char *ipc_tlv_value = tlv_buffer + TLV_OVERHEAD_SIZE;

    switch (ipc_tlv) {

        case TLV_IPC_NET_UDP_SKT:
        {
            uint32_t *ip_addr = (uint32_t *)(ipc_tlv_value);
            *ip_addr = htonl(ipc_struct->netskt.ip_addr);
            uint16_t *port = (uint16_t *)(ipc_tlv_value + 4);
            *port = htons(ipc_struct->netskt.port);
        }
        break;
        case TLV_IPC_TYPE_CBK:
        {
            pub_sub_cbk_t *cbk = (pub_sub_cbk_t *)ipc_tlv_value;
            *cbk = ipc_struct->cbk.cbk;
        }
        break;
        case TLV_IPC_TYPE_MSGQ:
        {
            char *msgq_name = (char *)ipc_tlv_value;
            strncpy (msgq_name, ipc_struct->msgq.MsgQName, 
                sizeof(ipc_struct->msgq.MsgQName));
        }
        break;
        case TLV_IPC_TYPE_UXSKT:
        {
            char *uxskt_name = (char *)ipc_tlv_value;
            strncpy (uxskt_name, ipc_struct->uxskt.UnixSktName, 
                sizeof (ipc_struct->uxskt.UnixSktName));
        }
        break;
        default:
            printf ("Client : Error : Invalid IPC Type\n");
            free(subscriber_ipc_msg);
            return;
    }

    int rc = pub_sub_dispatch_cmsg (sock_fd, subscriber_ipc_msg);

    if (rc < 0) {
        printf("Client : Error : Send Failed, errno = %d\n", errno);
    }

    free(subscriber_ipc_msg);
}