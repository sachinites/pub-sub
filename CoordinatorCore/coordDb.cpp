#include <stdint.h>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <memory>
#include <concepts>
#include "../Libs/tlv.h"
#include "pubsub.h"
#include "CoordDb.h"
#include "../Common/comm-types.h"
#include "../Libs/PostgresLibpq/postgresLib.h"
#include "../Common/ipc_struct.h"

extern PGconn* gconn;

std::unordered_map<uint32_t , publisher_db_entry_t *>pub_db;
std::unordered_map<uint32_t , std::shared_ptr<subscriber_db_entry_t>>sub_db;
std::unordered_map<uint32_t , pub_sub_db_entry_t *>pub_sub_db;

template <typename Key, typename Value>
class CORDCRUDOperations {
public:

    static bool create(std::unordered_map<Key, Value>& db, Key key, Value value) {

        auto it = db.find(key);
        if (it != db.end()) {
            //throw std::runtime_error("Key already exist in the database for create.");
            return false;
        }
        db[key] = value;
        return true;
    }

    static Value read(std::unordered_map<Key, Value>& db, Key key) 
        requires (std::is_pointer_v<Value>)
    {
        auto it = db.find(key);
        if (it != db.end()) {
            return it->second;
        }
        return nullptr; 
    }

    static Value read(std::unordered_map<Key, Value>& db, Key key) 
        requires (!std::is_pointer_v<Value>)
    {
        auto it = db.find(key);
        if (it != db.end()) {
            return it->second;
        }
        return Value();
    }

    static bool update(std::unordered_map<Key, Value*>& db, Key key, Value newValue) {

        auto it = db.find(key);
        if (it != db.end()) {
            it->second = newValue;
            return true;
        } else {

        }
        return false;
    }

    static Value remove(std::unordered_map<Key, Value>& db, Key key) 
        requires (std::is_pointer_v<Value>)
    {
        Value ret = nullptr;
        auto it = db.find(key);
        if (it != db.end()) {
            ret = it->second;
            db.erase(it);
        } else {
            throw std::runtime_error("Key not found in the database for deletion.");
        }
        return ret;
    }

    static Value remove(std::unordered_map<Key, Value>& db, Key key) 
        requires (!std::is_pointer_v<Value>)
    {
        Value ret;
        auto it = db.find(key);
        if (it != db.end()) {
            ret = it->second;
            db.erase(it);
            return ret;
        } else {
            throw std::runtime_error("Key not found in the database for deletion.");
            return Value();
        }
    }

    // Display , Since each Value Type needs to be printed differently, we use template specialization
    // to specialize display function for different Value data types.
    static void display(const std::unordered_map<Key, publisher_db_entry_t *>& db) {
        for (const auto& entry : db) {
            printf ("Publisher ID : %u | ", entry.first);
            printf ("Publisher Name : %s\n", entry.second->pub_name);
        }
    }

    static void display(const std::unordered_map<Key, subscriber_db_entry_t *>& db) {
        for (const auto& entry : db) {
            printf ("Subscriber ID : %u\n", entry.first);
            printf ("Subscriber Name : %s\n", entry.second->sub_name);
        }
    }

    static void display(const std::unordered_map<Key, pub_sub_db_entry_t *>& db) {
        for (const auto& entry : db) {
            printf ("Msg Code : %u\n", entry.first);
            printf ("Subscriber IDs : ");
            for (size_t i = 0; i < entry.second->subscriber_ids.size(); ++i) {
                printf ("%u  ", entry.second->subscriber_ids[i]);
            }
            printf ("\n");
        }
    }

};

bool 
publisher_publish_msg (uint32_t pub_id, 
                                    uint32_t published_msg_id) {

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t *>::\
                                read(pub_db, pub_id);

    if (!PubEntry) return false;

    int i; 

    for (i = 0; i < MAX_PUBLISHED_MSG; i++) {
        if (PubEntry->published_msg_ids[i] == published_msg_id) return false;
    }

    for (i = 0; i < MAX_PUBLISHED_MSG; i++) {
        if (PubEntry->published_msg_ids[i]) continue;
        PubEntry->published_msg_ids[i] = published_msg_id;

        printf ("Coordinator : Publisher %s published message %u Successfully\n", PubEntry->pub_name, published_msg_id);

        /* Update PUB-DB table with new publish msg ID*/
        char sql_query[256];
        snprintf (sql_query, sizeof(sql_query), 
                  "UPDATE %s.%s SET PUBLISHED_MSG_IDS[%d] = %u WHERE PUBID = %u;",
                    COORD_SCHEMA_NAME, PUB_TABLE_NAME, i, published_msg_id, pub_id);
        PGresult *res = PQexec(gconn, sql_query);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            printf("Coordinator : Error: Failed to update PUB-DB table with new published msg ID\n");
        }
        PQclear(res);
        return true;
    }

    return false;
}

bool 
publisher_unpublish_msg (uint32_t pub_id, 
                                            uint32_t published_msg_id) {

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t *>::
                                read(pub_db, pub_id);

    if (!PubEntry) return false;

    int i;

    for (i = 0; i < MAX_PUBLISHED_MSG; i++) {
        if (PubEntry->published_msg_ids[i] == published_msg_id) break;
    }

    if (i == MAX_PUBLISHED_MSG) return false;

    PubEntry->published_msg_ids[i] = 0;

    /* Update PUB-DB table with new publish msg ID*/

    char sql_query[256];
    snprintf (sql_query, sizeof(sql_query), 
              "UPDATE %s.%s SET PUBLISHED_MSG_IDS[%d] = %u WHERE PUBID = %u;",
                COORD_SCHEMA_NAME, PUB_TABLE_NAME,
              i, 0, pub_id);

    PGresult *res = PQexec(gconn, sql_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Coordinator : Error: Failed to update PUB-DB table with new published msg ID\n");
    }
    PQclear(res);
    return true;
}


publisher_db_entry_t *
publisher_db_create (uint32_t pub_id, 
                                    char *pub_name) {

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t *>::
        read(pub_db, pub_id);

    if (PubEntry)
        return PubEntry;

    PubEntry = new publisher_db_entry_t;
    memset (PubEntry, 0, sizeof(*PubEntry));

    strncpy(PubEntry->pub_name, pub_name, sizeof(PubEntry->pub_name));
    PubEntry->publisher_id = pub_id;
    CORDCRUDOperations<uint32_t, publisher_db_entry_t *>::
        create(pub_db, pub_id, PubEntry);

    /* Insert entry into PUB-DB SQL Table */
    char sql_query[256];
    snprintf (sql_query, sizeof(sql_query), 
                  "INSERT INTO %s.%s (PUBNAME, PUBID, NUM_MSG_PUBLISHED, PUBLISHED_MSG_IDS) "
                  "VALUES ('%s', %u, %u, ARRAY[0]);",
                    COORD_SCHEMA_NAME, PUB_TABLE_NAME,
                  PubEntry->pub_name, PubEntry->publisher_id, 0);

    PGresult *res = PQexec(gconn, sql_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Coordinator : Error: Failed to insert Publisher %s into PUB-DB table, error-code = %d, %s\n", 
        PubEntry->pub_name, PQresultStatus(res), PQerrorMessage(gconn));
    }
    PQclear(res);
    return PubEntry;
}

void
publisher_db_delete (uint32_t pub_id) {

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t *>::
        read(pub_db, pub_id);

    if (!PubEntry) return;

    CORDCRUDOperations<uint32_t, publisher_db_entry_t *>::remove(pub_db, pub_id);

    /* Delete entry from PUB-DB SQL Table */
    char sql_query[256];
    snprintf (sql_query, sizeof(sql_query), 
                  "DELETE FROM %s.%s WHERE PUBID = %u;",
                    COORD_SCHEMA_NAME, PUB_TABLE_NAME,
                  pub_id);

    PGresult *res = PQexec(gconn, sql_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Coordinator : Error: Failed to delete Publisher %u from PUB-DB table\n", pub_id);
    }
    PQclear(res);

    /* It is not referenced by shared ptr , so delete it explicitely*/
    delete PubEntry;
}

  
/* Subscriber Related Operations */
std::shared_ptr<subscriber_db_entry_t> 
subscriber_db_create (uint32_t sub_id, 
                                    char *sub_name) {

    std::shared_ptr<subscriber_db_entry_t> SubEntry = 
        CORDCRUDOperations<uint32_t, std::shared_ptr<subscriber_db_entry_t>>::
        read(sub_db, sub_id);

    if (SubEntry)   
        return SubEntry;

    SubEntry = std::make_shared<subscriber_db_entry_t>();
    strncpy(SubEntry->sub_name, sub_name, sizeof(SubEntry->sub_name));
    SubEntry->subscriber_id = sub_id;
    CORDCRUDOperations<uint32_t, std::shared_ptr<subscriber_db_entry_t>>::
        create(sub_db, sub_id, SubEntry);
    
    /* Insert entry into SUB-DB SQL Table */
    char sql_query[256];
    snprintf (sql_query, sizeof(sql_query), 
                  "INSERT INTO %s.%s (SUBNAME, SUBID, NUM_MSG_RECEIVED, SUBSCRIBED_MSG_IDS) "
                  "VALUES ('%s', %u, %u, ARRAY[0]);",
                    COORD_SCHEMA_NAME, SUB_TABLE_NAME,
                  SubEntry->sub_name, SubEntry->subscriber_id, 0);

    PGresult *res = PQexec(gconn, sql_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Coordinator : Error: Failed to insert Subscriber %s into SUB-DB table\n", SubEntry->sub_name);
    }
    PQclear(res);
    return SubEntry;
}

void
subscriber_db_delete (uint32_t sub_id) {

    std::shared_ptr<subscriber_db_entry_t> SubEntry = 
        CORDCRUDOperations<uint32_t, std::shared_ptr<subscriber_db_entry_t>>::
        read(sub_db, sub_id);

    if (!SubEntry) return;

    CORDCRUDOperations<uint32_t, std::shared_ptr<subscriber_db_entry_t>>::
        remove(sub_db, sub_id);


    if (SubEntry->ipc_struct.netskt.sock_fd) {
        close (SubEntry->ipc_struct.netskt.sock_fd);
    }   

    /* Delete entry from SUB-DB SQL Table */
    char sql_query[256];
    snprintf (sql_query, sizeof(sql_query), 
                  "DELETE FROM %s.%s WHERE SUBID = %u;",
                    COORD_SCHEMA_NAME, SUB_TABLE_NAME,
                  sub_id);

    PGresult *res = PQexec(gconn, sql_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Coordinator : Error: Failed to delete Subscriber %u from SUB-DB table\n", sub_id);
    }
    PQclear(res);
}

bool 
subscriber_subscribe_msg (uint32_t sub_id, 
                                            uint32_t msg_id) {

    auto SubEntry = CORDCRUDOperations<uint32_t, std::shared_ptr<subscriber_db_entry_t>>::
        read(sub_db, sub_id);

    if (!SubEntry) {
        printf("Coordinator : Error: Subscriber ID %u not found\n", sub_id);
        return false;
    }

    int i;

    for (i = 0; i < MAX_SUBSCRIBED_MSG; i++) {
        if (SubEntry->subscriber_msg_ids[i] == msg_id) {
            printf("Coordinator : Error: Subscriber [%s,%u] already subscribed to msg %u\n", SubEntry->sub_name, SubEntry->subscriber_id, msg_id);
        }
    }

    for (i = 0; i < MAX_SUBSCRIBED_MSG; i++) {

        if (SubEntry->subscriber_msg_ids[i]) continue;

        SubEntry->subscriber_msg_ids[i] = msg_id;

        printf ("Coordinator : Subscriber [%s,%u] Subscribed to msg %u Successfully\n",
            SubEntry->sub_name, SubEntry->subscriber_id, msg_id);   

        /* Update SUB-DB table with new subscribed msg ID*/
        char sql_query[256];
        snprintf (sql_query, sizeof(sql_query), 
                  "UPDATE %s.%s SET SUBSCRIBED_MSG_IDS[%d] = %u WHERE SUBID = %u;",
                    COORD_SCHEMA_NAME, SUB_TABLE_NAME,
                  i, msg_id, sub_id);

        PGresult *res = PQexec(gconn, sql_query);

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            printf("Coordinator : Error: Failed to update SUB-DB table with new subscribed msg ID\n");
        
        }
        PQclear(res);
        pub_sub_db_create (msg_id, SubEntry);
        return true;
    }
    return false;
 }

bool 
subscriber_unsubscribe_msg (uint32_t sub_id, 
                                                uint32_t msg_id) {

    std::shared_ptr<subscriber_db_entry_t> SubEntry = 
        CORDCRUDOperations<uint32_t, std::shared_ptr<subscriber_db_entry_t>>::
        read(sub_db, sub_id);

    if (!SubEntry) return false;

    int i;

    for (i = 0; i < MAX_SUBSCRIBED_MSG; i++) {
        if (SubEntry->subscriber_msg_ids[i] == msg_id) break;
    }

    if (i == MAX_SUBSCRIBED_MSG) return false;

    SubEntry->subscriber_msg_ids[i] = 0;

    printf ("Coordinator : Subscriber [%s,%u] Unsubscribed from msg %u Successfully\n",
        SubEntry->sub_name, SubEntry->subscriber_id, msg_id);

    /* Update SUB-DB table with new subscribed msg ID*/
    char sql_query[256];
    snprintf (sql_query, sizeof(sql_query), 
              "UPDATE %s.%s SET SUBSCRIBED_MSG_IDS[%d] = %u WHERE SUBID = %u;",
                COORD_SCHEMA_NAME, SUB_TABLE_NAME,
              i, 0, sub_id);

    PGresult *res = PQexec(gconn, sql_query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        printf("Coordinator : Error: Failed to update SUB-DB table with new subscribed msg ID\n");
    }

    PQclear(res);
    pub_sub_db_delete (msg_id, sub_id);
    return true;
}

bool 
coordinator_process_subscriber_ipc_subscription (
        uint32_t sub_id, 
        cmsg_t *cmsg) {

    assert (cmsg->msg_type == SUBS_TO_COORD);
    assert (cmsg->sub_msg_type == SUB_MSG_IPC_CHANNEL_ADD);

    auto SubEntry = CORDCRUDOperations<uint32_t, std::shared_ptr<subscriber_db_entry_t>>::
        read(sub_db, sub_id);

    if (!SubEntry) {
        
        printf("Coordinator : Error: Subscriber ID %u not found\n", sub_id);
        return false;
    }

    if (SubEntry->ipc_type != IPC_TYPE_NONE) {
        printf("Coordinator : Error: Subscriber [%s,%u] IPC Channel Already Exists\n", SubEntry->sub_name, SubEntry->subscriber_id);
        return false;
    }

    if (cmsg->tlv_buffer_size == 0) {
        printf("Coordinator : Error: Subscriber IPC Channel TLV Contains no IPC Data\n");
        return false;
    }

    char *tlv_buffer = (char *)cmsg->msg;
    size_t tlv_bufer_size = cmsg->tlv_buffer_size;
    uint8_t tlv_data_len = 0;
    uint8_t tlv_type;
    char *tlv_value;

    ITERATE_TLV_BEGIN(tlv_buffer, tlv_type, tlv_data_len, tlv_value, tlv_bufer_size) {

        switch (tlv_type) {

            case TLV_IPC_NET_UDP_SKT:
            {
                uint32_t ip_addr = *(uint32_t *)  (tlv_value);
                ip_addr = htonl(ip_addr);
                uint16_t port = *(uint16_t *) (tlv_value + 4);
                port = htons(port);
                printf("Coordinator : Subscriber [%s,%u] IPC Channel Add : IP Address %u, Port %u\n", 
                    SubEntry->sub_name, SubEntry->subscriber_id, ip_addr, port);
                SubEntry->ipc_type = IPC_TYPE_NETSKT;
                SubEntry->ipc_struct.netskt.ip_addr = ip_addr;
                SubEntry->ipc_struct.netskt.port = port;
                SubEntry->ipc_struct.netskt.transport_type = IPPROTO_UDP;

                /* Now update SUB-DB*/
                char sql_query[256];
                snprintf (sql_query, sizeof(sql_query), 
                          "UPDATE %s.%s SET IPC_DATA = ROW(%u, %u, %u, %u, %s, %s)::%s.ipc_struct"
                          " WHERE SUBID = %u;",
                            COORD_SCHEMA_NAME, SUB_TABLE_NAME,
                          SubEntry->ipc_type, SubEntry->ipc_struct.netskt.ip_addr, 
                          SubEntry->ipc_struct.netskt.port, SubEntry->ipc_struct.netskt.transport_type,
                          "''", "''", COORD_SCHEMA_NAME, sub_id);
                
                printf ("Executing SQL Query : %s\n", sql_query);

                PGresult *res = PQexec(gconn, sql_query);
                if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                    printf("Coordinator : Error: Failed to update SUB-DB table with new IPC Channel\n");
                    PQclear(res);
                    return false;
                }
                PQclear(res);
                return true;
            }
            break;

            case TLV_IPC_TYPE_CBK:
            {
                uintptr_t fn_ptr = *(uintptr_t *)tlv_value;
                SubEntry->ipc_type = IPC_TYPE_CBK;
                SubEntry->ipc_struct.cbk.cbk = (pub_sub_cbk_t )fn_ptr;
                printf("Coordinator : Subscriber [%s,%u] IPC Channel Add : Callback Function %p\n", 
                    SubEntry->sub_name, SubEntry->subscriber_id, SubEntry->ipc_struct.cbk.cbk);

                /* Now update SUB-DB*/
                char sql_query[256];
                snprintf (sql_query, sizeof(sql_query), 
                          "UPDATE %s.%s SET IPC_DATA = ROW(%u, %u, %u, %u, %s, %s)::%s.ipc_struct"
                          " WHERE SUBID = %u;",
                            COORD_SCHEMA_NAME, SUB_TABLE_NAME,
                          SubEntry->ipc_type, 0, 0, 0, "''", "''", COORD_SCHEMA_NAME, sub_id);
                
                printf ("Executing SQL Query : %s\n", sql_query);

                PGresult *res = PQexec(gconn, sql_query);
                if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                    printf("Coordinator : Error: Failed to update SUB-DB table with new IPC Channel\n");
                    PQclear(res);
                    return false;
                }
                PQclear(res);
                return true;
            }

            default:
                printf("Coordinator : Error: Unknown TLV Type %u in Subscriber IPC Channel TLV\n", tlv_type);
                return false;
        }

    } ITERATE_TLV_END;

    return true;
}


pub_sub_db_entry_t *
pub_sub_db_create (uint32_t msg_id, 
                                std::shared_ptr<subscriber_db_entry_t> SubEntry) {

    auto PubSubEntry = pub_sub_db_get (msg_id);

    if (!PubSubEntry) {

        PubSubEntry = new pub_sub_db_entry_t;
        PubSubEntry->publish_msg_code = msg_id;
        PubSubEntry->subscribers.push_back(SubEntry);
        
        CORDCRUDOperations<uint32_t, pub_sub_db_entry_t *>::
            create(pub_sub_db, msg_id, PubSubEntry);

        /* Insert entry into PUB-SUB-DB SQL Table */
        char sql_query[256];
        snprintf (sql_query, sizeof(sql_query), 
                  "INSERT INTO %s.%s (PUB_MSG_CODE, SUBSCRIBER_IDS) "
                  "VALUES (%u, ARRAY[%u]);",
                    COORD_SCHEMA_NAME, PUB_SUB_TABLE_NAME,
                  PubSubEntry->publish_msg_code, SubEntry->subscriber_id);

        PGresult *res = PQexec(gconn, sql_query);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            printf("Coordinator : Error: Failed to insert PUB-SUB-DB table with new subscriber ID, error-code = %u, %s\n", PQresultStatus(res), PQerrorMessage(gconn));
        }
        PQclear(res);
        return PubSubEntry;
    }

    for (size_t i = 0; i < PubSubEntry->subscribers.size(); ++i)
    {
        if (PubSubEntry->subscribers[i]->subscriber_id == SubEntry->subscriber_id)
        {
            printf("Coordinator : INFO : Subscriber ID [%s, %u] already subscribed to Msg Code %u\n", SubEntry->sub_name, SubEntry->subscriber_id, msg_id);
            return PubSubEntry;
        }
    }

    PubSubEntry->subscribers.push_back(SubEntry);

    /* Update the SQL DB now*/
    char sql_query[256];
    snprintf(sql_query, sizeof(sql_query),
             "UPDATE %s.%s SET SUBSCRIBER_IDS = ARRAY_APPEND(SUBSCRIBER_IDS, %u) "
             "WHERE PUB_MSG_CODE = %u;",
             COORD_SCHEMA_NAME, PUB_SUB_TABLE_NAME,
             SubEntry->subscriber_id, msg_id);

    PGresult *res = PQexec(gconn, sql_query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        printf("Coordinator : Error: Failed to update PUB-SUB-DB table with new subscriber ID\n");
    }

    PQclear(res);
    return PubSubEntry;
}


void 
pub_sub_db_delete (uint32_t msg_id, 
                                    uint32_t sub_id) {
    
    auto PubSubEntry = pub_sub_db_get (msg_id);

    if (!PubSubEntry) return;

    for (size_t i = 0; i < PubSubEntry->subscribers.size(); ++i) {

        if (PubSubEntry->subscribers[i]->subscriber_id == sub_id) {

            PubSubEntry->subscribers.erase(PubSubEntry->subscribers.begin() + i);

             /* Update the SQL DB now*/
            char sql_query[256];
            snprintf (sql_query, sizeof(sql_query), 
                      "UPDATE %s.%s SET SUBSCRIBER_IDS = ARRAY_REMOVE(SUBSCRIBER_IDS, %u) WHERE PUB_MSG_CODE = %u;",
                        COORD_SCHEMA_NAME, PUB_SUB_TABLE_NAME,
                      sub_id, msg_id);

            PGresult *res = PQexec(gconn, sql_query);
            if (PQresultStatus(res) != PGRES_COMMAND_OK) {
                printf("Coordinator : Error: Failed to update PUB-SUB-DB table with new subscriber ID\n");
            }

            PQclear(res);
            return;
        }
    }
}

pub_sub_db_entry_t *
pub_sub_db_get (uint32_t msg_id) {

    pub_sub_db_entry_t *res =  CORDCRUDOperations<uint32_t, pub_sub_db_entry_t *>::
        read(pub_sub_db, msg_id);
    return res;
}