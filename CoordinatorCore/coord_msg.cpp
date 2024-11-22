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


cmsg_t *
coordinator_process_publisher_msg (cmsg_t *msg, size_t bytes_read) {

    cmsg_t *reply_msg;

    assert (msg->msg_type == PUB_TO_COORD);

    memset (&reply_msg, 0, sizeof (reply_msg));

    msg->msg_id = coord_generate_id();

    switch (msg->sub_msg_type) {

        case SUB_MSG_ADD:
        {
            bool rc = publisher_db_add_new_publshed_msg (msg->id.publisher_id, msg->msg_id);
            if (!rc) {
                printf ("Coordinator : Error : New Msg Publishing Failed by Published ID %u\n", msg->id.publisher_id);
            }
        }
        break;
        case SUB_MSG_DELETE:
        break;
        case SUB_MSG_REGISTER:
        {
            /* New Publisher Registration */
            char *tlv_buffer = (char *)msg->msg;
            size_t tlv_bufer_size = msg->tlv_buffer_size;
            uint8_t tlv_data_len = 0;
            char *pub_name = tlv_buffer_get_particular_tlv (
                                        tlv_buffer, tlv_bufer_size, 
                                        TLV_CODE_NAME, 
                                        &tlv_data_len);

            if (!pub_name) {
                printf ("Coordinator : Error : Publisher Registration : Publisher Name TLV Missing\n");
                reply_msg = (cmsg_t *)calloc (1, sizeof (*reply_msg) + TLV_OVERHEAD_SIZE);
                reply_msg->msg_id = coord_generate_id();
                reply_msg->msg_type = COORD_TO_PUB;
                reply_msg->sub_msg_type = SUB_MSG_ERROR;
                reply_msg->msg_code =  ERROR_TLV_MISSING;
                reply_msg->id.publisher_id = 0;
                reply_msg->tlv_buffer_size = TLV_OVERHEAD_SIZE;
                char *tlv_buffer = reply_msg->msg;
                tlv_buffer_insert_tlv (tlv_buffer, TLV_CODE_NAME, 0, NULL);
                reply_msg->msg_size = TLV_OVERHEAD_SIZE;
                return reply_msg;
            }
            publisher_db_create (coord_generate_id() ,  pub_name);
        }
        break;
        case SUB_MSG_UNREGISTER:
        {
            /* Existing Publisher Withdrawl*/

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
    
    return NULL;
}

void
coordinator_process_subscriber_msg (cmsg_t *msg, size_t bytes_read) {


}

