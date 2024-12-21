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
    cmsg_t cmsg;;

    coordinator_register (sock_fd, "Pub1", PUB_TO_COORD);
    rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
                            0, NULL, NULL);
    printf ("Pub Msg ID allocated = %u\n", cmsg.id.publisher_id);
    
    int pub_id = cmsg.id.publisher_id;

    /* Let Publisher Publish three message types */
    publisher_publish (sock_fd, pub_id, 100);
    publisher_publish (sock_fd, pub_id, 101);
    publisher_publish (sock_fd, pub_id, 102);

    printf ("Press any key to send Publisher Msg to Coordinator \n");
    getchar ();

    const char *msg = "Hello World, I am a Publisher";

    cmsg_t *data_cmsg = cmsg_data_prepare (
                                        PUB_TO_COORD, 
                                        SUB_MSG_DATA, 
                                        100, 
                                        true, 1, TLV_DATA_128);

    data_cmsg->id.publisher_id = pub_id;
    data_cmsg->msg_id = 0; // This will be assigned by coordinator
    data_cmsg->priority = CMSG_PR_MEDIUM;
    data_cmsg->meta_data.flags = 0;

    uint8_t tlv_data_len = 0;
    char *data_tlv_value = tlv_buffer_get_particular_tlv (
                                                (char *)data_cmsg->msg,
                                                data_cmsg->tlv_buffer_size,
                                                TLV_DATA_128, &tlv_data_len);

    strncpy (data_tlv_value, msg, strlen(msg));

    pub_sub_dispatch_cmsg (sock_fd, data_cmsg);
    free (data_cmsg);

    printf ("Press any key to quit Publisher \n");
    getchar ();

    publisher_unpublish (sock_fd, pub_id, 100);
    publisher_unpublish (sock_fd, pub_id, 101);
    publisher_unpublish (sock_fd, pub_id, 102);
    coordinator_unregister (sock_fd, pub_id, PUB_TO_COORD);
    
    close (sock_fd);

    return 0;
}