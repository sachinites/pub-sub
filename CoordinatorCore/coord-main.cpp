#include <stdint.h>
#include <pthread.h>
#include <unordered_map>
#include <assert.h>
#include <stdlib.h>
#include "mqueue.h"
#include "pubsub.h"
#include "../Libs/PostgresLibpq/postgresLib.h"
#include "../Common/comm-types.h"

static std::unordered_map<uint32_t , publisher_db_entry_t *>pub_db;
static std::unordered_map<uint32_t , subscriber_db_entry_t *>sub_db;
static std::unordered_map<uint32_t , pub_sub_db_entry_t *>pub_sub_db;
PGconn* gconn;

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

/* This function does the following : 
    1. Create a new DB user called  "Coordinator"
    2. Create a new DB called COORD_DB_NAME
    3. Assign new user created in step 1 all priveleges on this new DB created in step 2

    We need to re-config postgresDB before running this fn as follows :
     
    1. Open file /etc/postgresql/<version>/main/pg_hba.conf   (in my case, version was 16)
    2.  Change 'peer' to 'md5' in below line
            local   all             postgres                                peer
        
    3. Add below line immediately after the line you modified above 
            local   all             coordinator                             md5
    4. Add below extra line 
    # IPv4 local connections:
    host    all             all             127.0.0.1/32            scram-sha-256
    host    all             coordinator     127.0.0.1/32            md5                <<<< new line added
    
    5. restart DB :  sudo systemctl restart postgresql
*/

static void 
coordinator_init_sql_db() {

    /* Create SQL DB*/
    int rc;
    PGconn* conn;
    PGresult* sql_query_result;
    const char *coord_db_name = COORD_DB_NAME;
    const char *user_name = COORD_DB_USER;
    char sql_query[256];

    /* Create a new user of postgres DB. Here new user will be 'Coordinator'.  */
    rc = postgresql_create_new_user(NULL, user_name);
    if (rc == PGSQL_FAILED) {
        assert(0);
    }

    /* Establish database connection for super-user 'postgres' . Only Super user
      can create new users or new Databases or do any privileged changes to DB*/
    conn = postgres_get_user_connection(NULL,  "postgres", "postgres");
    assert(conn);

    /* Create a new DB. If already exist, do nothing */
    rc = postgresql_create_new_database(NULL, coord_db_name);
    if (rc == PGSQL_FAILED) {
        assert(0);
    }

    /* Assigned this new DB 'coord_db_name' to new user 'Coordinator' with all privileges*/
    assert (postgresql_database_assign_user (conn, user_name, coord_db_name) != PGSQL_FAILED);

    /* Get rid of the superuser 'postgres' connection, re-establish the new connection for new user 
        with db-name also included as a paramater*/
    PQfinish(conn);

    snprintf (sql_query, sizeof(sql_query), 
                 "host=localhost user=%s dbname=%s password=%s",  
                  user_name, coord_db_name, user_name);

    gconn = PQconnectdb(sql_query);

    if (PQstatus(gconn) != CONNECTION_OK) {
        printf("Error: PQconnectdb Failed\n");
        printf("User: %s, Database: %s, Password: %s\n", user_name, coord_db_name, user_name);
        printf("Error Message: %s\n", PQerrorMessage(gconn));
        PQfinish(gconn); 
    }

 }


static void 
coordinator_recv_msg_listen() {

    mqd_t mq;
    int ret;
    struct mq_attr attr;
    fd_set readfds;
    int nfds;
    char buffer[COORD_RECV_Q_MAX_MSG_SIZE];

    attr.mq_flags = 0;
    attr.mq_maxmsg = COORD_RECV_Q_MAX_MSGS;
    attr.mq_msgsize = COORD_RECV_Q_MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mq = mq_open(COORD_MSGQ_NAME, O_RDONLY | O_CREAT, 0666, &attr);
    if (mq == (mqd_t) -1) {
        perror("mq_open");
        exit(1);
    }

    nfds = mq + 1;  // The nfds argument to select is the highest fd + 1

    while (1) {

        FD_ZERO(&readfds);
        FD_SET(mq, &readfds); 
        ret = select(nfds, &readfds, NULL, NULL, NULL);
        
        if (ret == -1) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(mq, &readfds)) {
            
            ssize_t bytes_read;
            bytes_read = mq_receive(mq, buffer, sizeof(buffer), NULL);
            if (bytes_read == -1) {
                exit(1);
            }
            
            buffer[bytes_read] = '\0';
            cmsg_t *msg = (cmsg_t *)buffer;
            
            switch (msg->msg_type) {
                case PUB_TO_COORD:
                    printf("Received message from publisher\n");
                    break;
                case SUBS_TO_COORD:
                    printf("Received message from subscriber\n");
                    break;
                default:
                    printf("Received unknown message type\n");
                    break;
            }
        }
    }
    /* Unreachable code*/
}

static void *
coordinator_main_fn (void *arg) {

    coordinator_init_sql_db();
    coord_init_publisher_table();
    coord_init_subscriber_table();
    coord_init_pub_sub_table() ;
    coordinator_recv_msg_listen();

    return NULL;
}

void 
coordinator_main() {

    static pthread_t thread; 
    pthread_create (&thread, NULL, coordinator_main_fn, NULL);  
}