#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "../Libs/tlv.h"
#include "client.h"

/* Returns the TLV to be used to encode the ipc channel */
static int 
ipv_type_to_tlv_type (ipc_type_t ipc_type) {

    switch (ipc_type) {
        case IPC_TYPE_MSGQ:
            return TLV_IPC_TYPE_MSGQ;
        case IPC_TYPE_UXSKT:
            return TLV_IPC_TYPE_UXSKT;
        case IPC_TYPE_NETSKT:
            return TLV_IPC_NET_UDP_SKT;
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
                                    char *entity_name, 
                                    msg_type_t msg_type) {

    cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (*msg) + TLV_OVERHEAD_SIZE + TLV_CODE_NAME_LEN);
    msg->msg_id =0;     // This will be assigned by coordinator
    msg->msg_type = msg_type;
    msg->sub_msg_type = SUB_MSG_REGISTER;
    msg->id.publisher_id = 0;   // This will be assigned by coordinator
    msg->id.subscriber_id = 0;  // This will be assigned by coordinator
    msg->tlv_buffer_size = TLV_OVERHEAD_SIZE +  TLV_CODE_NAME_LEN;
    char *tlv_buffer = (char *)msg->msg;
    tlv_buffer_insert_tlv (tlv_buffer, TLV_CODE_NAME, TLV_CODE_NAME_LEN, entity_name);
    int rc = pub_sub_dispatch_cmsg (sock_fd, msg);
    if (rc < 0) {
        printf ("Client : Error : Send Failed, errno = %d\n", errno);
    }
    //cmsg_debug_print (msg);
    free(msg);
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
publisher_publish (int sock_fd, uint32_t pub_id, uint32_t msg_code) {

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
subscriber_subscribe_ipc_channel (int sock_fd, 
                                uint32_t sub_id, 
                                ipc_type_t ipc_type, 
                                ipc_struct_t *ipc_struct) {

    
    int ipc_tlv = ipv_type_to_tlv_type (ipc_type);
    
    if (!ipc_tlv) {
        printf ("Client : Error : Invalid IPC Type\n");
        return;
    }

    uint8_t tlv_size = tlv_data_len (ipc_tlv);

    cmsg_t *subscriber_ipc_msg = cmsg_data_prepare (
                                                            SUBS_TO_COORD, 
                                                            SUB_MSG_IPC_CHANNEL_ADD,
                                                            0, /* Not required */
                                                            true, 1, ipc_tlv);

    subscriber_ipc_msg->id.subscriber_id = sub_id;
    subscriber_ipc_msg->msg_id = 0; // This will be assigned by coordinator

    uint8_t tlv_data_len = 0;

    char *ipc_skt_tlv = tlv_buffer_get_particular_tlv (
                                                (char *)subscriber_ipc_msg->msg,
                                                subscriber_ipc_msg->tlv_buffer_size,
                                                ipc_tlv, &tlv_data_len);

    switch (ipc_tlv) {

        case TLV_IPC_NET_UDP_SKT:
        {
            uint32_t *ip_addr = (uint32_t *)(ipc_skt_tlv);
            *ip_addr = htonl(ipc_struct->netskt.ip_addr);
            uint16_t *port = (uint16_t *)(ipc_skt_tlv + 4);
            *port = htons(ipc_struct->netskt.port);
        }
        break;
        default:
            printf ("Client : Error : Invalid IPC Type\n");
            free(subscriber_ipc_msg);
            return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(COORD_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(COORD_IP_ADDR);

    int rc = sendto(sock_fd, (char *)subscriber_ipc_msg, 
                            sizeof(*subscriber_ipc_msg) + subscriber_ipc_msg->tlv_buffer_size, 
                            0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

    if (rc < 0) {
        printf("Client : Error : Send Failed, errno = %d\n", errno);
    }

    free(subscriber_ipc_msg);
}