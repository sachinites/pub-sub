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