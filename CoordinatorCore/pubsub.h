#ifndef __PUB_SUB__
#define __PUB_SUB__

#include <stdint.h>
#include <vector>
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

} subscriber_db_entry_t ;


/* Pub-Sub DB*/
typedef struct pub_sub_db_entry_ {

    uint32_t publish_msg_code; 
    std::vector <subscriber_db_entry_t *> subscribers;

} pub_sub_db_entry_t;

#endif 