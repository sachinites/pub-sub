#ifndef __PUB_SUB__
#define __PUB_SUB__

#include <stdint.h>
#include <vector>
#include <cstring>
#include <memory>
#include "CoordIpc.h"
#include "Config.h"


/* Publisher DBs*/
typedef struct publisher_db_entry_ {

    /* To be informed by Client-APP */
    char pub_name[64];

    /* To be internally generated , key */
    uint32_t publisher_id; 

    /*Array of Msgs published by this publisher */
    uint32_t published_msg_ids[MAX_PUBLISHED_MSG];

    void clear() {
        pub_name[0] = '\0';
        publisher_id = 0;
        memset (published_msg_ids, 0, sizeof (published_msg_ids));
    }
    
} publisher_db_entry_t; 


/* Subscriber DBs*/
typedef struct subscriber_db_entry_ {

    /* To be informed by Client-APP */
    char sub_name[64];

    /* To be internally generated , key */
    uint32_t subscriber_id; 

    /*Array of Msgs subscriber is interested */
    uint32_t subscriber_msg_ids[MAX_SUBSCRIBED_MSG];

    ipc_type_t ipc_type;
    ipc_struct_t ipc_struct;

    void clear() {
        sub_name[0] = '\0';
        subscriber_id = 0;
        memset (subscriber_msg_ids, 0, sizeof (subscriber_msg_ids));
        ipc_type = IPC_TYPE_NONE;
        memset (&ipc_struct, 0, sizeof (ipc_struct));
    }

    /* Add destructor */
    ~subscriber_db_entry_() {
        if (ipc_type == IPC_TYPE_NETSKT &&
                ipc_struct.netskt.sock_fd > 0) {
            close (ipc_struct.netskt.sock_fd);
        }
    }

} subscriber_db_entry_t ;


/* Pub-Sub DB*/
typedef struct pub_sub_db_entry_ {

    uint32_t publish_msg_code; 
    std::vector <std::shared_ptr<subscriber_db_entry_t>> subscribers;

    void clear () {

        publish_msg_code = 0;
        subscribers.clear();
    }

} pub_sub_db_entry_t;

#endif 