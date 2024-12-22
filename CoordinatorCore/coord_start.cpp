#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "mqueue.h"
#include "pubsub.h"
#include "../Libs/PostgresLibpq/postgresLib.h"
#include "../Common/comm-types.h"
#include "../Common/cmsgOp.h"

extern cmsg_t *
coordinator_process_publisher_msg (cmsg_t *msg, size_t bytes_read);
extern cmsg_t *
coordinator_process_subscriber_msg (cmsg_t *msg, size_t bytes_read);
extern void 
coordinator_fork_distribution_threads();

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

   char sql_query[256];

   snprintf (sql_query, sizeof (sql_query), "CREATE TABLE %s.%s ("
                            "PUBNAME TEXT NOT NULL,"
                            "PUBID INT PRIMARY KEY NOT NULL,"
                            "NUM_MSG_PUBLISHED INT NOT NULL,"
                            "PUBLISHED_MSG_IDS INT[] NOT NULL);", 
                            COORD_SCHEMA_NAME, PUB_TABLE_NAME);

    PGresult *res = PQexec(gconn, sql_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Warning: Failed to create Publisher table, error code = %d, %s\n",           
            PQresultStatus(res), PQerrorMessage(gconn));
    }
   PQclear(res);
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

   char sql_query[256];

  snprintf (sql_query, sizeof (sql_query), 
    "CREATE TYPE %s.ipc_struct AS ("
        "ipc_type INT,"
        "netskt_ip_addr INT,"
        "netskt_port INT,"
        "netskt_transport_type INT,"
        "msgq_name TEXT,"
        "uxskt_name TEXT);", COORD_SCHEMA_NAME); 
    PGresult *res = PQexec(gconn, sql_query);
    PQclear(res);

    snprintf (sql_query, sizeof (sql_query), "CREATE TABLE %s.%s ("
                            "SUBNAME TEXT NOT NULL,"
                            "SUBID INT PRIMARY KEY NOT NULL,"
                            "NUM_MSG_RECEIVED INT NOT NULL,"
                            "SUBSCRIBED_MSG_IDS INT[] NOT NULL,"
                            "IPC_DATA %s.ipc_struct);",
                            COORD_SCHEMA_NAME, SUB_TABLE_NAME,
                            COORD_SCHEMA_NAME);
    res = PQexec(gconn, sql_query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Warning: Failed to create Subscriber table, error code = %d, %s\n",           
            PQresultStatus(res), PQerrorMessage(gconn));
    }
    PQclear(res);
}

extern pthread_spinlock_t pub_sub_db_lock;
static void 
coord_init_pub_sub_table() {

    /*
        DB-name : PUB-SUB
        Fields:
            Published Msg Code
            List of Subscriber IDs
    */

   char sql_query[256];
    snprintf (sql_query, sizeof (sql_query), "CREATE TABLE %s.%s ("
                            "PUB_MSG_CODE INT PRIMARY KEY NOT NULL,"
                            "SUBSCRIBER_IDS INT[] NOT NULL);",
                            COORD_SCHEMA_NAME, PUB_SUB_TABLE_NAME);
    
    PGresult *res = PQexec(gconn, sql_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Warning: Failed to create Pub-Sub table, error code = %d, %s\n",           
            PQresultStatus(res), PQerrorMessage(gconn));
    }

    PQclear(res);
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

    /* create Schema for the new user */
    sprintf(sql_query, "create schema %s authorization %s", COORD_SCHEMA_NAME, user_name);
    sql_query_result = PQexec(conn, sql_query);

    if (PQresultStatus(sql_query_result) != PGRES_COMMAND_OK)
    {
        printf ("Warning : Creating schema authorization %s Failed, error_code = %d, %s\n",
            user_name, PQresultStatus(sql_query_result), PQerrorMessage(conn));
        PQclear(sql_query_result);
    }

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
coordinator_reply (int sock_fd, cmsg_t *reply_msg, struct sockaddr_in *client_addr) {

    size_t msg_size_to_send = sizeof (*reply_msg) + reply_msg->tlv_buffer_size;
    int rc = sendto(sock_fd, (char *)reply_msg, msg_size_to_send, 0,
                    (struct sockaddr *)client_addr, sizeof(struct sockaddr));
    if (rc < 0) {
        printf("Coordinator : Error : Feeback Reply to Subscriber Failed\n");
    }
}

static void 
coordinator_recv_msg_listen() {

    int ret;
    fd_set readfds;
    int sock_fd, addr_len, opt = 1;
    sub_msg_type_t sub_msg_code;
    cmsg_t *reply_msg = NULL;
    static char buffer[COORD_RECV_Q_MAX_MSG_SIZE];
    struct sockaddr_in server_addr,
                                  client_addr;
    
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP )) == -1)
    {
        printf("Coordinator : Error : Listener Socket creation failed\n");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(COORD_UDP_PORT);
    server_addr.sin_addr.s_addr = htonl(COORD_IP_ADDR); 
    addr_len = sizeof(struct sockaddr);

    if (setsockopt(sock_fd, SOL_SOCKET,
                   SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("Coordinator : Error : SO_REUSEADDR failed\n");
        close (sock_fd);
        exit(1);
    }

    if (setsockopt(sock_fd, SOL_SOCKET,
                   SO_REUSEPORT, (char *)&opt, sizeof(opt)) < 0)
    {
        printf("Coordinator : Error : SO_REUSEPORT failed\n");
        close (sock_fd);
        exit(1);
    }

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        printf("Coordinator : Error : bind failed\n");
        close (sock_fd);
        exit(1);
    }

    printf("Coordinator : Listening for Requests. . .\n");

    while (1)
    {

        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);
        //FD_SET(STDIN_FILENO, &readfds);

        select(sock_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(sock_fd, &readfds))
        {
            ssize_t bytes_read;

            while (1)
            {
                bytes_read = recvfrom(sock_fd, buffer, sizeof(buffer),
                                      0, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);

                if (bytes_read <= 0) break;
                
                buffer[bytes_read] = '\0';
                cmsg_t *msg = (cmsg_t *)buffer;

                switch (msg->msg_type)
                {
                case PUB_TO_COORD:

                    if (msg->id.publisher_id) {
                        printf("Coordinator : Received message from publisher id = %u\n", msg->id.publisher_id);
                    }
                    else {
                        printf("Coordinator : Received message from New publisher\n");
                    }
                    
                    cmsg_debug_print (msg);

                    reply_msg = coordinator_process_publisher_msg(msg, bytes_read);
                    if (reply_msg)
                    {
                        coordinator_reply(sock_fd, reply_msg, &client_addr);
                        free(reply_msg);
                    }
                    break;
                case SUBS_TO_COORD:

                    if (msg->id.subscriber_id) {
                        printf("Coordinator : Received message from Subscriber id = %u\n", msg->id.subscriber_id);
                    }
                    else {
                        printf("Coordinator : Received message from New Subscriber\n");
                    }

                    cmsg_debug_print (msg);
                    
                    reply_msg = coordinator_process_subscriber_msg(msg, bytes_read);
                    if (reply_msg)
                    {
                        coordinator_reply(sock_fd, reply_msg, &client_addr);
                        free(reply_msg);
                    }
                    break;
                default:
                    printf("Coordinator : Received unknown message type\n");
                    break;
                }
            }
        }
        else if (FD_ISSET(STDIN_FILENO, &readfds))
        {

            fgets(buffer, sizeof(buffer), stdin);
            if (buffer[0] == '\n')
                continue;
            printf("echo : %s", buffer);
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
    coordinator_fork_distribution_threads();
    coordinator_recv_msg_listen();

    return NULL;
}

void 
coordinator_main() {

    static pthread_t thread; 
    pthread_create (&thread, NULL, coordinator_main_fn, NULL);  
}
