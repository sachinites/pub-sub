#include <stdint.h>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <string.h>
#include "pubsub.h"

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
publisher_db_add_new_publshed_msg (uint32_t pub_id, 
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
        return true;
    }

    return false;
}

publisher_db_entry_t *
publisher_db_create (uint32_t pub_id, 
                                    char *pub_name) {

    auto PubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t>::
        read(pub_db, pub_id);

    if (PubEntry)
        return PubEntry;

    PubEntry = new publisher_db_entry_t;
    strncpy(PubEntry->pub_name, pub_name, sizeof(PubEntry->pub_name));
    PubEntry->publisher_id = pub_id;
    CORDCRUDOperations<uint32_t, publisher_db_entry_t>::
        create(pub_db, pub_id, PubEntry);
    return PubEntry;
}


#if 0

// Main function for demonstration
int main() {
    // Create entries
    auto* pubEntry = new publisher_db_entry_t{"PublisherName", 42, {1, 2, 3, 4}};
    auto* subEntry = new subscriber_db_entry_t{"SubscriberName", 43, {1, 2, 3, 4}};
    auto* pubSubEntry = new pub_sub_db_entry_t{4, {101, 102, 103}};

    // Create or Update entries in databases
    CORDCRUDOperations<uint32_t, publisher_db_entry_t>::create(pub_db, 1, pubEntry);
    CORDCRUDOperations<uint32_t, subscriber_db_entry_t>::create(sub_db, 2, subEntry);
    CORDCRUDOperations<uint32_t, pub_sub_db_entry_t>::create(pub_sub_db, 3, pubSubEntry);

    // Read entries
    auto readPubEntry = CORDCRUDOperations<uint32_t, publisher_db_entry_t>::read(pub_db, 1);
    if (readPubEntry) {
        std::cout << "Read publisher_db_entry_t: " << readPubEntry->pub_name << "\n";
    }

    // Update entry
    auto* updatedPubEntry = new publisher_db_entry_t{99};
    CORDCRUDOperations<uint32_t, publisher_db_entry_t>::update(pub_db, 1, updatedPubEntry);

    // Display databases
    std::cout << "Publisher DB:\n";
    CORDCRUDOperations<uint32_t, publisher_db_entry_t>::display(pub_db);

    // Delete an entry
    CORDCRUDOperations<uint32_t, publisher_db_entry_t>::remove(pub_db, 1);

    // Display databases after deletion
    std::cout << "Publisher DB after deletion:\n";
    CORDCRUDOperations<uint32_t, publisher_db_entry_t>::display(pub_db);

    // Clean up remaining entries
    for (auto& entry : sub_db) {
        delete entry.second;
    }
    for (auto& entry : pub_sub_db) {
        delete entry.second;
    }

    return 0;
}

#endif 