#include <stdint.h>
#include "../Common/comm-types.h"

typedef struct publisher_db_entry_ publisher_db_entry_t; 

/* Publisher DB operations */
bool 
publisher_db_add_new_publshed_msg (uint32_t pub_id, 
                                                                uint32_t published_msg_id);

publisher_db_entry_t *
publisher_db_create (uint32_t pub_id, 
                                    char *pub_name);