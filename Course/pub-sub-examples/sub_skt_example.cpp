#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../Common/comm-types.h"
#include "../clientlib/client.h"

// udp port no
#define SUB_SKT_UDP_PORT_NO 50001
uint16_t argument_port_no = 0;
static char buffer[1024];

void *
sub_skt_example(void *arg) {

    int sock_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock_fd == -1) {
        printf ("Error : Socket Creation Failed\n");
        return 0;
    }

    struct sockaddr_in self_addr;
    self_addr.sin_family = AF_INET;
    if (argument_port_no) {
        self_addr.sin_port = htons (argument_port_no);
    }
    else {
        self_addr.sin_port = htons (SUB_SKT_UDP_PORT_NO);
    }
    
    self_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock_fd, (struct sockaddr *)&self_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Coordinator : Error : bind failed\n");
        close (sock_fd);
        exit(1);
    }

    int rc;
    cmsg_t cmsg;

    coordinator_register (sock_fd, "Sub1", SUBS_TO_COORD);

    rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
            0, NULL, NULL);

    printf ("Sub Msg ID allocated = %u\n", cmsg.id.subscriber_id);

    int sub_id = cmsg.id.subscriber_id;

    printf ("Press any key to subscribe message 100\n");
    getchar ();
    subscriber_subscribe (sock_fd, sub_id, 100);
    printf ("Press any key to subscribe message 200\n");
    getchar ();
    subscriber_subscribe (sock_fd, sub_id, 200);



    /* Report the channel of communication */
    ipc_struct_t ipc_struct;
    ipc_struct.netskt.ip_addr = INADDR_ANY;
    ipc_struct.netskt.port = htons(self_addr.sin_port) ;
    printf ("Press any key to report IPC channel SKT to Coordinator \n");
    getchar();
    subscriber_subscribe_ipc_channel (sock_fd, sub_id, IPC_TYPE_NETSKT, &ipc_struct);

    while (1) {
        
        printf ("Subscriber now waiting for msgs from Coordinator\n");

        rc = recvfrom (sock_fd, (char *)buffer, sizeof (buffer), 
                            0, NULL, NULL);
        cmsg_t *recv_msg = (cmsg_t *)buffer;
        
        printf ("Subscriber : Msg recvd from Coordinator\n");
        cmsg_debug_print (recv_msg);

        char *tlv_buffer = recv_msg->tlv_buffer;
        uint16_t tlv_buffer_size = recv_msg->tlv_buffer_size;
        uint8_t tlv_data_len;
        char *tlv_value = tlv_buffer_get_particular_tlv (
                tlv_buffer, tlv_buffer_size, 
                TLV_DATA_128, &tlv_data_len);
        
        if (!tlv_value) {
            printf ("Error : No TLV_DATA found in TLV buffer\n");
            continue;
        }

        printf ("Data Message recvd by Subscriber [%u] is : %s\n\n", sub_id, tlv_value);
    }

    printf ("Press any key to UnSubscribe message 100\n");
    getchar ();
    subscriber_unsubscribe (sock_fd, sub_id, 100);
    printf ("Press any key to UnSubscribe message 200\n");
    getchar ();
    subscriber_unsubscribe (sock_fd, sub_id, 200);

    printf ("Press any key to Unregister Subscriber \n");
    getchar ();

    coordinator_unregister (sock_fd, sub_id, SUBS_TO_COORD);

    close (sock_fd);
    return NULL;
}