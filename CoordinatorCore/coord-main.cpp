#include <stdint.h>
#include <pthread.h>
#include "pubsub.h"

#include <unordered_map>

static std::unordered_map<uint32_t , publisher_db_entry_t *>pub_db;
static std::unordered_map<uint32_t , subscriber_db_entry_t *>sub_db;
static std::unordered_map<uint32_t , pub_sub_db_entry_t *>pub_sub_db;

extern void 
coordinator_main();

static void 
coord_init_publisher_table() {

    /* 
    DB-name : PUB-DB
    Fields: 
        publisher name 
        publisher id
        Number of mssages published
        List of Msg IDs Published
    */

}

static void 
coord_init_subscriber_table() {

    /*
        DB-name : SUB-DB
        Fields:
            subscriber name
            Subscriber id
            Number of Messages Recvd
            List of Msg IDs Subscribed
    */
}


static void 
coord_init_pub_sub_table() {

    /*
        DB-name : PUB-SUB
        Fields:
            Published Msg Code
            List of Subscriber IDs
    */
}

static void 
coordinator_init_sql_db() {

 }

static void 
coordinator_listen_msgQ() {

    
}

static void *
coordinator_main_fn (void *arg) {

    coordinator_init_sql_db();
    coord_init_publisher_table();
    coord_init_subscriber_table();
    coord_init_pub_sub_table() ;
    coordinator_listen_msgQ();

    return NULL;
}

void 
coordinator_main() {

    static pthread_t thread; 
    pthread_create (&thread, NULL, coordinator_main_fn, NULL);  
}