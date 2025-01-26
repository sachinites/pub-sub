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
#define PUB_SKT_UDP_PORT_NO 50000
uint16_t argument_port_no = 0;

void *
pub_skt_example(void *arg) {

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
        self_addr.sin_port = htons (PUB_SKT_UDP_PORT_NO);
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

    coordinator_register (sock_fd, "Pub1", PUB_TO_COORD);

    rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
            0, NULL, NULL);

    printf ("Pub Msg ID allocated = %u\n", cmsg.id.publisher_id);

    int pub_id = cmsg.id.publisher_id;

    printf ("Press ay key to publish message 100\n");
    getchar ();
    publisher_publish (sock_fd, pub_id, 100);
    printf ("Press any key to publish message 200\n");
    getchar();

    /*
    Let Publisher send a sample Data to the Coordinator of Msg Type 100
    */
   cmsg_t *data_cmsg = cmsg_data_prepare2 (PUB_TO_COORD,
            SUB_MSG_DATA,
            100,
            TLV_OVERHEAD_SIZE + tlv_data_len (TLV_DATA_128));

    data_cmsg->id.publisher_id = pub_id;
    data_cmsg->priority = CMSG_PR_HIGH;
    data_cmsg->ref_count = 1;
    char *tlv_buffer = data_cmsg->tlv_buffer;
    tlv_buffer_insert_tlv (tlv_buffer, 
                        data_cmsg->tlv_buffer_size, 
                        TLV_DATA_128, (char *)"Sample Data Sent by PUB1");

    getchar ();
    publisher_publish (sock_fd, pub_id, 200);
    printf ("Press any key to Unpublish message 100\n");
    getchar ();
    publisher_unpublish (sock_fd, pub_id, 100);
    printf ("Press any key to unpublish message 200\n");
    getchar ();
    publisher_unpublish (sock_fd, pub_id, 200);

    printf ("Press any key to Unregister Publisher \n");
    getchar ();

    coordinator_unregister (sock_fd, pub_id, PUB_TO_COORD);

    close (sock_fd);
    return NULL;
}