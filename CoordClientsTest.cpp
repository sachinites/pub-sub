
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "../clientlib/client.h"

int 
main (int arhc, char **argv) {
    
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock_fd == -1) {
        printf ("Error : Socket Creation Failed\n");
        return -1;
    }

    struct sockaddr_in self_addr;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(40000);
    self_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(COORD_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(COORD_IP_ADDR);

    if (bind(sock_fd, (struct sockaddr *)&self_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Coordinator : Error : bind failed\n");
        close (sock_fd);
        exit(1);
    }

    int rc;
    cmsg_t cmsg;

#if 1
    coordinator_register (sock_fd, "Pub1", PUB_TO_COORD);
    rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
                            0, NULL, NULL);
    printf ("Pub Msg ID allocated = %u\n", cmsg.id.publisher_id);
    int pub_id = cmsg.id.publisher_id;

    publisher_publish (sock_fd, pub_id, 100);

    coordinator_register (sock_fd, "Sub1", SUBS_TO_COORD);
    rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
                            0, NULL, NULL);
    printf ("Subs Msg ID allocated = %u\n", cmsg.id.subscriber_id);
    int sub_id = cmsg.id.subscriber_id;

    subscriber_subscribe  (sock_fd, sub_id, 100);


    /* Now let subscriber inform how it wants to recv the msg from coord*/
    cmsg_t *subscriber_ipc_msg = cmsg_data_prepare (
                                                        SUBS_TO_COORD, 
                                                        SUB_MSG_IPC_CHANNEL_ADD,
                                                        0, /* Not required */
                                                        true, 1, TLV_IPC_NET_UDP_SKT);

    subscriber_ipc_msg->id.subscriber_id = sub_id;
    subscriber_ipc_msg->msg_id = 0; // This will be assigned by coordinator
            
    uint8_t tlv_data_len = 0;
    char *ipc_skt_tlv = tlv_buffer_get_particular_tlv (
                                                (char *)subscriber_ipc_msg->msg,
                                                subscriber_ipc_msg->tlv_buffer_size,
                                                TLV_IPC_NET_UDP_SKT, &tlv_data_len);

    uint32_t *ip_addr = (uint32_t *)  (ipc_skt_tlv);
    *ip_addr = htonl(INADDR_ANY);
    uint16_t *port = (uint16_t *) (ipc_skt_tlv + 4);
    *port = htons(40000);

    rc = sendto(sock_fd, (char *)subscriber_ipc_msg, 
                            sizeof(*subscriber_ipc_msg) + subscriber_ipc_msg->msg_size, 
                            0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

    /* Now let publisher send the data message */
    cmsg_t *data_cmsg = cmsg_data_prepare (
                                        PUB_TO_COORD, 
                                        SUB_MSG_DATA, 
                                        100, 
                                        false, 0);

    data_cmsg->id.publisher_id = pub_id;
    data_cmsg->msg_id = 0; // This will be assigned by coordinator
    data_cmsg->priority = CMSG_PR_MEDIUM;
    data_cmsg->meta_data.flags = 0;

    rc = sendto(sock_fd, (char *)data_cmsg, 
                            sizeof(*data_cmsg) + data_cmsg->msg_size, 
                            0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

    rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
                            0, NULL, NULL);

    cmsg_debug_print (&cmsg);

    return 0;
    
#endif 

#if 0
    coordinator_register (sock_fd, "Pub2", PUB_TO_COORD);
    coordinator_register (sock_fd, "Pub3", PUB_TO_COORD);
    coordinator_register (sock_fd, "Sub1", SUBS_TO_COORD);
    coordinator_register (sock_fd, "Sub2", SUBS_TO_COORD);
    coordinator_register (sock_fd, "Sub3", SUBS_TO_COORD);
#endif 

    while (1) {
        memset (&cmsg, 0, sizeof (cmsg));
        int rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
            0, NULL, NULL);
        printf ("Msg ID allocated = %u\n", cmsg.id.publisher_id);

        #if 0
        coordinator_unregister (sock_fd,
            cmsg.id.publisher_id,
            cmsg.msg_type == COORD_TO_PUB ? \
                PUB_TO_COORD : SUBS_TO_COORD);
        #endif 

        if (cmsg.msg_type == COORD_TO_SUBS) {
            
            subscriber_subscribe  (sock_fd, cmsg.id.subscriber_id, 100);
            //subscriber_subscribe  (sock_fd, cmsg.id.subscriber_id, 101);
            //subscriber_subscribe  (sock_fd, cmsg.id.subscriber_id, 102);

            cmsg_t *subscriber_ipc_msg = cmsg_data_prepare (
                                                        SUBS_TO_COORD, 
                                                        SUB_MSG_IPC_CHANNEL_ADD,
                                                        0, /* Not required */
                                                        true, 1, TLV_IPC_NET_UDP_SKT);

            subscriber_ipc_msg->id.subscriber_id = cmsg.id.subscriber_id;
            subscriber_ipc_msg->msg_id = 0; // This will be assigned by coordinator
            
            uint8_t tlv_data_len = 0;
            char *ipc_skt_tlv = tlv_buffer_get_particular_tlv (
                                                (char *)subscriber_ipc_msg->msg,
                                                subscriber_ipc_msg->tlv_buffer_size,
                                                TLV_IPC_NET_UDP_SKT, &tlv_data_len);

            uint32_t *ip_addr = (uint32_t *)  (ipc_skt_tlv);
            *ip_addr = htonl(16909060);
            uint16_t *port = (uint16_t *) (ipc_skt_tlv + 4);
            *port = htons(40000);

            cmsg_debug_print (subscriber_ipc_msg);
            int rc = sendto(sock_fd, (char *)subscriber_ipc_msg, 
                                    sizeof(*subscriber_ipc_msg) + subscriber_ipc_msg->msg_size, 
                                    0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

            if (rc < 0) {
                printf("Client : Error : Send Failed, errno = %d\n", errno);
            }
            
            free(subscriber_ipc_msg);
        }
        else if (cmsg.msg_type == COORD_TO_PUB) {
            publisher_publish (sock_fd, cmsg.id.publisher_id, 100);
            //publisher_publish (sock_fd, cmsg.id.publisher_id, 101);
            //publisher_publish (sock_fd, cmsg.id.publisher_id, 102);
        }
    }
    return 0;
}