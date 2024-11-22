#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <cstdio>
#include "../Libs/tlv.h"
#include "../Common/comm-types.h"
#include "CoordDb.h"

template <typename Key, typename Value>
class CORDCRUDOperations;

static uint32_t 
coord_generate_id () {

    static uint32_t id = 0;
    return ++id;
}

void
coordinator_process_publisher_msg (cmsg_t *msg, size_t bytes_read) {

    assert (msg->msg_type == PUB_TO_COORD);

    msg->msg_id = coord_generate_id();

    switch (msg->sub_msg_type) {

        case SUB_MSG_ADD:
        {
            bool rc = publisher_db_add_new_publshed_msg (msg->publisher_id, msg->msg_id);
            if (!rc) {
                printf ("Coordinator : Error : New Msg Publishing Failed by Published ID %u\n", msg->publisher_id);
            }
        }
        break;
        case SUB_MSG_DELETE:
        break;
        case SUB_MSG_REGISTER:
        {
            /* New Publisher Registration */
            char *tlv_buffer = (char *)msg->msg;
            size_t tlv_bufer_size = msg->msg_size;
            uint8_t tlv_data_len = 0;
            char *pub_name = tlv_buffer_get_particular_tlv (
                                        tlv_buffer, tlv_bufer_size, 
                                        TLV_CODE_NAME, 
                                        &tlv_data_len);
            publisher_db_create (coord_generate_id() ,  pub_name);
        }
        break;
        case SUB_MSG_UNREGISTER:
        {

        }
        break;
        case SUB_MSG_REQUEST_ACK:
        break;
        case SUB_MSG_ACK_MSG:
        break;
        case SUB_MSG_SUBCRIBER_LIST:
        break;
        case SUB_MSG_INFORM_NEW_SUBS:
        break;
        default:    ;
    }
    
}

void
coordinator_process_subscriber_msg (cmsg_t *msg, size_t bytes_read) {


}