#include <stdint.h>
#include "../Common/comm-types.h"

typedef struct publisher_db_entry_ publisher_db_entry_t; 
typedef struct cmsg_ cmsg_t;

/* Publisher DB operations */
publisher_db_entry_t *
publisher_db_create (uint32_t pub_id, 
                                    char *pub_name);

void
publisher_db_delete (uint32_t pub_id) ;

bool 
publisher_publish_msg (uint32_t pub_id, 
                                        uint32_t published_msg_id);

bool 
publisher_unpublish_msg (uint32_t pub_id, 
                                            uint32_t published_msg_id); 

/* Subscriber DB Operations */
typedef struct subscriber_db_entry_ subscriber_db_entry_t;

subscriber_db_entry_t *
subscriber_db_create (uint32_t sub_id, 
                                    char *sub_name);

void
subscriber_db_delete (uint32_t sub_id) ;

bool 
subscriber_subscribe_msg (uint32_t sub_id, 
                                            uint32_t msg_id);

bool 
subscriber_unsubscribe_msg (uint32_t sub_id, 
                                                uint32_t msg_id); 

bool 
coordinator_process_subscriber_ipc_subscription (
        uint32_t sub_id, cmsg_t *cmsg);


/* Operations on PUB-SUB DB */

typedef struct pub_sub_db_entry_ pub_sub_db_entry_t;

pub_sub_db_entry_t *
pub_sub_db_create (uint32_t msg_id, 
                                 subscriber_db_entry_t *SubEntry);

void 
pub_sub_db_delete (uint32_t msg_id, 
                                    uint32_t sub_id);

pub_sub_db_entry_t *
pub_sub_db_get (uint32_t msg_id);