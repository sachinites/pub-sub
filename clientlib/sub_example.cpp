#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include "client.h"

int 
main (int argc, char **argv) {
    
    int sock_fd;

    if (argc != 3) {
        printf ("Usage : %s <Self IP Address> <Self UDP Port Number>\n", 
            argv[0]);
        return -1;
    }

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock_fd == -1) {
        printf ("Error : Socket Creation Failed\n");
        return -1;
    }

    struct sockaddr_in self_addr;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = htons(atoi(argv[2]));
    
    inet_pton(AF_INET, argv[1], &self_addr.sin_addr);
    //self_addr.sin_addr.s_addr = htonl(INADDR_ANY);

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

    /* Let Publisher Publish three message types */
    subscriber_subscribe  (sock_fd, sub_id, 100);
    subscriber_subscribe  (sock_fd, sub_id, 101);
    subscriber_subscribe  (sock_fd, sub_id, 104);

    /* Report the channel of communication */
    ipc_struct_t ipc_struct;
    ipc_struct.netskt.ip_addr = self_addr.sin_addr.s_addr;  // 127.0.0.1
    ipc_struct.netskt.port = htons(self_addr.sin_port);
    subscriber_subscribe_ipc_channel (sock_fd, sub_id, IPC_TYPE_NETSKT, &ipc_struct);

    while (1) {
        
        rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
                            0, NULL, NULL);
        printf ("Mesg Recvd : %s\n", (char *)cmsg.msg);
    }

    close (sock_fd);
    return 0;
}