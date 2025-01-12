#include "../Common/comm-types.h"
#include "../Common/cmsgOp.h"

extern cmsg_t *
coordinator_process_publisher_msg (cmsg_t *msg, size_t bytes_read);

extern cmsg_t *
coordinator_process_subscriber_msg (cmsg_t *msg, size_t bytes_read);

void
pub_sub_send_ips (cmsg_t *cmsg) {

    cmsg_set_ips (cmsg);

    switch (cmsg->msg_type)
    {
    case PUB_TO_COORD:
    {
        coordinator_process_publisher_msg(cmsg, sizeof(cmsg) + cmsg->tlv_buffer_size);
    }
    break;
    case SUBS_TO_COORD:
    {
        coordinator_process_subscriber_msg(cmsg, sizeof(cmsg) + cmsg->tlv_buffer_size);
    }
    break;
    default:
        printf("Coordinator : Error : Invalid Message Type\n");
    }
}