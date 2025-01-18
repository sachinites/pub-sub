#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include "../Common/ipc_struct.h"
#include "../Common/cmsgOp.h"
#include "../Common/comm-types.h"
#include "client.h"
#include "../Common/clientcommon.h"

void *
pub_cbk_example (void *_ipc_struct) {
    
    int sock_fd;

    sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sock_fd == -1) {
        printf ("Error : Socket Creation Failed\n");
        return 0;
    }

    struct sockaddr_in self_addr;
    self_addr.sin_family = AF_INET;
    self_addr.sin_port = 0;  // will be assigned by bind
    self_addr.sin_addr.s_addr = htonl(INADDR_ANY); // bind to all local ip addresses

    if (bind(sock_fd, (struct sockaddr *)&self_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Coordinator : Error : bind failed\n");
        close (sock_fd);
        exit(1);
    }

    int rc; 
    cmsg_t cmsg;

    coordinator_register (sock_fd, "Pub_CBK", PUB_TO_COORD);
    rc = recvfrom (sock_fd, (char *)&cmsg, sizeof (cmsg), 
                            0, NULL, NULL);
    printf ("Pub_CBK : Pub Msg ID allocated = %u\n", cmsg.id.publisher_id);
    
    int pub_id = cmsg.id.publisher_id;

    /* Let Publisher Publish three message types */
    publisher_publish  (sock_fd, pub_id, 103);
    publisher_publish  (sock_fd, pub_id, 104);
    publisher_publish  (sock_fd, pub_id, 105);
    close (sock_fd);

    /* Send the IPS now*/
    const char *msg = "Hello World, I am a CBK Publisher";

    cmsg_t *data_cmsg = cmsg_data_prepare (
                                        PUB_TO_COORD, 
                                        SUB_MSG_DATA, 
                                        101, 
                                        true, 1, TLV_DATA_128);
    
    data_cmsg->id.publisher_id = pub_id;
    data_cmsg->msg_id = 0; // This will be assigned by coordinator
    data_cmsg->priority = CMSG_PR_MEDIUM;
    data_cmsg->meta_data.flags = 0;

    uint8_t tlv_data_len = 0;
    char *data_tlv_value = tlv_buffer_get_particular_tlv (
                                                (char *)data_cmsg->tlv_buffer,
                                                data_cmsg->tlv_buffer_size,
                                                TLV_DATA_128, &tlv_data_len);

    strncpy (data_tlv_value, msg, strlen(msg));
    pub_sub_send_ips (data_cmsg);   

    return 0;
}
