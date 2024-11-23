
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "Libs/tlv.h"
#include "Common/comm-types.h"

static void 
 coordinator_register (int sock_fd, char *entity_name, 
                                    msg_type_t msg_type, 
                                    sub_msg_type_t sub_msg_type) {

    cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (*msg) + TLV_OVERHEAD_SIZE + TLV_CODE_NAME_LEN);
    msg->msg_id =0; // This will be assigned by coordinator
    msg->msg_type = msg_type;
    msg->sub_msg_type = sub_msg_type;
    msg->id.publisher_id = 0; // This will be assigned by coordinator
    msg->id.subscriber_id = 0; // This will be assigned by coordinator
    msg->tlv_buffer_size = TLV_OVERHEAD_SIZE +  TLV_CODE_NAME_LEN;
    msg->msg_size = msg->tlv_buffer_size;
    char *tlv_buffer = (char *)msg->msg;
    tlv_buffer_insert_tlv (tlv_buffer, TLV_CODE_NAME, TLV_CODE_NAME_LEN, entity_name);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(COORD_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(COORD_IP_ADDR);
    int rc = sendto (sock_fd, (char *)msg, sizeof (*msg) + msg->msg_size, 0, 
        (struct sockaddr *)&server_addr, sizeof (struct sockaddr));

    if (rc < 0) {
        printf ("Client : Error : Send Failed, errno = %d\n", errno);
    }
    free(msg);
 }

static void 
 coordinator_unregister (int sock_fd, uint32_t entity_id,
                                    msg_type_t msg_type, 
                                    sub_msg_type_t sub_msg_type) {

    cmsg_t *msg = (cmsg_t *)calloc (1, sizeof (*msg));
    msg->msg_id =0; // This will be assigned by coordinator
    msg->msg_type = msg_type;
    msg->sub_msg_type = sub_msg_type;
    msg->id.publisher_id = entity_id;
    msg->id.subscriber_id = entity_id;
    msg->tlv_buffer_size = 0;
    msg->msg_size = 0;
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(COORD_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(COORD_IP_ADDR);
    int rc = sendto (sock_fd, (char *)msg, sizeof (*msg), 0, 
        (struct sockaddr *)&server_addr, sizeof (struct sockaddr));
    if (rc < 0) {
        printf("Client : Error : Send Failed, errno = %d\n", errno);
    }
    free(msg);
}


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

    if (bind(sock_fd, (struct sockaddr *)&self_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Coordinator : Error : bind failed\n");
        close (sock_fd);
        exit(1);
    }

    coordinator_register (sock_fd, "Pub1", PUB_TO_COORD, SUB_MSG_REGISTER);
    coordinator_register (sock_fd, "Pub2", PUB_TO_COORD, SUB_MSG_REGISTER);
    coordinator_register (sock_fd, "Pub3", PUB_TO_COORD, SUB_MSG_REGISTER);
    coordinator_register (sock_fd, "Sub1", SUBS_TO_COORD, SUB_MSG_REGISTER);
    coordinator_register (sock_fd, "Sub2", SUBS_TO_COORD, SUB_MSG_REGISTER);
    coordinator_register (sock_fd, "Sub3", SUBS_TO_COORD, SUB_MSG_REGISTER);

    cmsg_t cmsg;
    while (1) {
        memset (&cmsg, 0, sizeof (cmsg));
        int rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
            0, NULL, NULL);
        printf ("Msg ID allocated = %u\n", cmsg.id.publisher_id);
        coordinator_unregister (sock_fd,
            cmsg.id.publisher_id,
            cmsg.msg_type == COORD_TO_PUB ? \
                PUB_TO_COORD : SUBS_TO_COORD, 
            SUB_MSG_UNREGISTER);
    }
    return 0;
}