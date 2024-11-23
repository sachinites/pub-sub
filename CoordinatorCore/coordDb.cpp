#include <stdint.h>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "../Libs/tlv.h"
#include "pubsub.h"
#include "../Common/comm-types.h"
#include "../Libs/PostgresLibpq/postgresLib.h"
#include "CoordIpc.h"

extern PGconn* gconn;

static std::unordered_map<uint32_t , publisher_db_entry_t *>pub_db;
static std::unordered_map<uint32_t , subscriber_db_entry_t *>sub_db;
static std::unordered_map<uint32_t , pub_sub_db_entry_t *>pub_sub_db;

// CRUD template
template <typename Key, typename Value>
class CORDCRUDOperations {
public:
    // Create or Insert
    static bool create(std::unordered_map<Key, Value*>& db, Key key, Value* value) {

        auto it = db.find(key);
        if (it != db.end()) {
            //throw std::runtime_error("Key already exist in the database for create.");
            return false;
        }
        db[key] = value;
        return true;
    }

    // Read
    static Value* read(const std::unordered_map<Key, Value*>& db, Key key) {

        auto it = db.find(key);
        if (it != db.end()) {
            return it->second;
        }
        return nullptr; // Return nullptr if key doesn't exist
    }

    // Update
    static bool update(std::unordered_map<Key, Value*>& db, Key key, Value* newValue) {

        auto it = db.find(key);
        if (it != db.end()) {
            it->second = newValue;
            return true;
        } else {
            //throw std::runtime_error("Key not found in the database for update.");
        }
        return false;
    }

    // Delete
    static Value* remove(std::unordered_map<Key, Value*>& db, Key key) {
        
        Value *ret = nullptr;
        auto it = db.find(key);
        if (it != db.end()) {
            db.erase(it);
            return ret;
        } else {
            throw std::runtime_error("Key not found in the database for deletion.");
        }
        return nullptr;
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

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t>::\
                                read(pub_db, pub_id);

    if (!PubEntry) return false;

    int i; 

    for (i = 0; i < MAX_PUBLISHED_MSG; i++) {
        if (PubEntry->published_msg_ids[i] == published_msg_id) return false;
    }

    for (i = 0; i < MAX_PUBLISHED_MSG; i++) {
        if (PubEntry->published_msg_ids[i]) continue;
        PubEntry->published_msg_ids[i] = published_msg_id;

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
        PQfinish(gconn);    
        return true;
    }

    return false;
}

bool 
publisher_unpublish_msg (uint32_t pub_id, 
                                            uint32_t published_msg_id) {

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t>::
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
    PQfinish(gconn);
    return true;
}


publisher_db_entry_t *
publisher_db_create (uint32_t pub_id, 
                                    char *pub_name) {

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t>::
        read(pub_db, pub_id);

    if (PubEntry)
        return PubEntry;

    PubEntry = new publisher_db_entry_t;
    memset (PubEntry, 0, sizeof(*PubEntry));

    strncpy(PubEntry->pub_name, pub_name, sizeof(PubEntry->pub_name));
    PubEntry->publisher_id = pub_id;
    CORDCRUDOperations<uint32_t, publisher_db_entry_t>::
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

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t>::
        read(pub_db, pub_id);

    if (!PubEntry) return;

    CORDCRUDOperations<uint32_t, publisher_db_entry_t>::
        remove(pub_db, pub_id);

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
}

  
/* Subscriber Related Operations */
subscriber_db_entry_t *
subscriber_db_create (uint32_t sub_id, 
                                    char *sub_name) {

    auto SubEntry = CORDCRUDOperations<uint32_t, subscriber_db_entry_t>::
        read(sub_db, sub_id);

    if (SubEntry)   
        return SubEntry;

    SubEntry = new subscriber_db_entry_t;
    memset (SubEntry, 0, sizeof(*SubEntry));

    strncpy(SubEntry->sub_name, sub_name, sizeof(SubEntry->sub_name));
    SubEntry->subscriber_id = sub_id;
    CORDCRUDOperations<uint32_t, subscriber_db_entry_t>::
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

    auto SubEntry = CORDCRUDOperations<uint32_t, subscriber_db_entry_t>::
        read(sub_db, sub_id);

    if (!SubEntry) return;

    CORDCRUDOperations<uint32_t, subscriber_db_entry_t>::
        remove(sub_db, sub_id);

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

    auto SubEntry = CORDCRUDOperations<uint32_t, subscriber_db_entry_t>::
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
        return true;
    }
    return false;
 }

bool 
subscriber_unsubscribe_msg (uint32_t sub_id, 
                                                uint32_t msg_id) {

    auto SubEntry = CORDCRUDOperations<uint32_t, subscriber_db_entry_t>::
        read(sub_db, sub_id);

    if (!SubEntry) return false;

    int i;

    for (i = 0; i < MAX_SUBSCRIBED_MSG; i++) {
        if (SubEntry->subscriber_msg_ids[i] == msg_id) break;
    }

    if (i == MAX_SUBSCRIBED_MSG) return false;

    SubEntry->subscriber_msg_ids[i] = 0;

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
    return true;
}

bool 
coordinator_process_subscriber_ipc_subscription (
        uint32_t sub_id, 
        cmsg_t *cmsg) {

    assert (cmsg->msg_type == SUBS_TO_COORD);
    assert (cmsg->sub_msg_type == SUB_MSG_IPC_CHANNEL_ADD);

    auto SubEntry = CORDCRUDOperations<uint32_t, subscriber_db_entry_t>::
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
            default:
                printf("Coordinator : Error: Unknown TLV Type %u in Subscriber IPC Channel TLV\n", tlv_type);
                return false;
        }

    } ITERATE_TLV_END;

    return true;
}