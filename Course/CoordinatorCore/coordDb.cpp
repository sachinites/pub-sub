#include <iostream>
#include <stdio.h>
#include <unordered_map>
#include <stdexcept>
#include "coordDb.h"
#include "pubsub.h"

std::unordered_map<uint32_t , publisher_db_entry_t *>pub_db;
std::unordered_map<uint32_t , std::shared_ptr<subscriber_db_entry_t>>sub_db;
std::unordered_map<uint32_t , pub_sub_db_entry_t *>pub_sub_db;

publisher_db_entry_t *
publisher_db_create (uint32_t pub_id, 
                     char *pub_name) {


    publisher_db_entry_t *pub_entry;

    /* Check if the publisher already exists */
    if (pub_db.find (pub_id) != pub_db.end()) {
        throw std::runtime_error ("Publisher already exists");
        return NULL;
    }
    
    pub_entry = new publisher_db_entry_t;
    if (pub_entry == NULL) {
        throw std::runtime_error ("Memory allocation failed for publisher entry");
    }

    pub_entry->publisher_id = pub_id;
    strncpy (pub_entry->pub_name, pub_name, 64);
    pub_db[pub_id] = pub_entry;
    return pub_entry;
}

void
publisher_db_delete (uint32_t pub_id) {

    if (pub_db.find (pub_id) == pub_db.end()) {
        throw std::runtime_error ("Publisher does not exist");
        return;
    }

    delete pub_db[pub_id];
    pub_db.erase (pub_id);
}


bool 
publisher_publish_msg (uint32_t pub_id, 
                       uint32_t published_msg_id) {


    publisher_db_entry_t* PubEntry = NULL;

    auto it = pub_db.find(pub_id);

    if (it == pub_db.end()) {
        return false;
    }

    PubEntry = it->second;

    int i; 

    for (i = 0; i < MAX_PUBLISHED_MSG; i++) {
        if (PubEntry->published_msg_ids[i] == published_msg_id) return false;
    }

    for (i = 0; i < MAX_PUBLISHED_MSG; i++) {
        if (PubEntry->published_msg_ids[i]) continue;
        PubEntry->published_msg_ids[i] = published_msg_id;

        printf ("Coordinator : Publisher %s published message %u Successfully\n", 
            PubEntry->pub_name, published_msg_id);
        return true;
    }

    return false;
}

bool 
publisher_unpublish_msg (uint32_t pub_id, 
                         uint32_t published_msg_id) {
    
    publisher_db_entry_t* PubEntry = NULL;

    auto it = pub_db.find(pub_id);

    if (it == pub_db.end()) {
        return false;
    }

    PubEntry = it->second;

    int i; 

    for (i = 0; i < MAX_PUBLISHED_MSG; i++) {
        if (PubEntry->published_msg_ids[i] == published_msg_id) {
            PubEntry->published_msg_ids[i] = 0;
            return true;
        }
    }

    return false;
}




/* Subscriber DB APIs*/

std::shared_ptr<subscriber_db_entry_t> 
subscriber_db_create(uint32_t sub_id, char* sub_name) {

    // Check if the subscriber already exists
    if (sub_db.find(sub_id) != sub_db.end()) {
        printf ("Subscriber with ID %u already exists.\n", sub_id);
        return nullptr;
    }

    // Create a new subscriber and insert into the database
    auto new_subscriber = std::make_shared<subscriber_db_entry_t>();
    new_subscriber->subscriber_id = sub_id;
    strncpy (new_subscriber->sub_name, sub_name, sizeof (new_subscriber->sub_name));
    sub_db[sub_id] = new_subscriber;
    return new_subscriber;
}

// Function to delete a subscriber
void subscriber_db_delete(uint32_t sub_id) {
    auto it = sub_db.find(sub_id);
    if (it != sub_db.end()) {
        sub_db.erase(it);
        printf("Subscriber with ID %u deleted.\n", sub_id);
    } else {
        printf("Subscriber with ID %u Not found.\n", sub_id);
    }
}

// Function to subscribe a message for a subscriber
bool 
subscriber_subscribe_msg(uint32_t sub_id, uint32_t msg_id) {

    subscriber_db_entry_t *SubEntry;

    auto it = sub_db.find(sub_id);

    if (it == sub_db.end()) {
        printf("%s() : Error : Subscriber with ID %u not found.\n", 
            __FUNCTION__, sub_id);
        return false;
    }

    // Add the message ID to the subscriber's subscribed messages
    SubEntry = it->second.get();

    for (int i = 0; i < MAX_SUBSCRIBED_MSG; i++) {
        if (SubEntry->subscriber_msg_ids[i]) continue;
        SubEntry->subscriber_msg_ids[i] = msg_id;

        printf ("Coordinator : Subscriber %s subscribed to message %u Successfully\n", 
            SubEntry->sub_name, msg_id);
        
        break;
    }

    /* Update publisher subscriber database also*/
    pub_sub_db_create (msg_id, it->second);

    return true;
}

// Function to unsubscribe a message for a subscriber
bool 
subscriber_unsubscribe_msg(uint32_t sub_id, uint32_t msg_id) {
    
    auto it = sub_db.find(sub_id);

    if (it == sub_db.end()) {

        printf("%s() : Error : Subscriber with ID %u not found.\n", 
            __FUNCTION__, sub_id);
        
        return false;
    }

    // Remove the message ID from the subscriber's subscribed messages
    auto& SubEntry = it->second;

    for (int i = 0; i < MAX_SUBSCRIBED_MSG; i++) {

        if (SubEntry->subscriber_msg_ids[i] == msg_id) {
            SubEntry->subscriber_msg_ids[i] = 0;
            break;
        }
    }

    /* Update publisher subscriber database also*/
    pub_sub_db_delete (msg_id, sub_id);

    return true;
}


// Function to create or update a pub_sub_db_entry_t in the database
pub_sub_db_entry_t* 
pub_sub_db_create (uint32_t msg_id,         
                                 std::shared_ptr<subscriber_db_entry_t> SubEntry) {

    auto it = pub_sub_db.find(msg_id);
    
    if (it == pub_sub_db.end()) {
        // Create a new entry
        pub_sub_db_entry_t* new_entry = new pub_sub_db_entry_t();
        new_entry->publish_msg_code = msg_id;
        new_entry->subscribers.push_back(SubEntry);
        pub_sub_db[msg_id] = new_entry;
        std::cout << "Created a new pub_sub_db entry for msg_id " << msg_id << "\n";
        return new_entry;
    }

    // Update existing entry
    pub_sub_db_entry_t* existing_entry = it->second;
    existing_entry->subscribers.push_back(SubEntry);
    std::cout << "Updated pub_sub_db entry for msg_id " << msg_id << "\n";
    return existing_entry;
}

// Function to delete a subscriber from the database
void pub_sub_db_delete (uint32_t msg_id, uint32_t sub_id) {

    auto it = pub_sub_db.find(msg_id);
    if (it == pub_sub_db.end()) {
        std::cout << "No pub_sub_db entry found for msg_id " << msg_id << "\n";
        return;
    }

    pub_sub_db_entry_t* entry = it->second;

    // Find and remove the subscriber
    auto& subscribers = entry->subscribers;
    for (auto iter = subscribers.begin(); iter != subscribers.end(); ++iter) {
        if ((*iter)->subscriber_id == sub_id) {
            subscribers.erase(iter);
            std::cout << "Subscriber with ID " << sub_id << " removed from msg_id " << msg_id << "\n";
            break;
        }
    }

    // If no more subscribers, delete the entry
    if (subscribers.empty()) {
        delete entry;
        pub_sub_db.erase(it);
        std::cout << "Deleted pub_sub_db entry for msg_id " << msg_id << "\n";
    }

}

// Function to get a pub_sub_db_entry_t from the database
pub_sub_db_entry_t* pub_sub_db_get(uint32_t msg_id) {
    auto it = pub_sub_db.find(msg_id);
    if (it == pub_sub_db.end()) {
        std::cout << "No pub_sub_db entry found for msg_id " << msg_id << "\n";
        return nullptr;
    }

    return it->second;
}

void
coord_db_display() {

    printf ("Publisher DB\n");
    for (auto it = pub_db.begin(); it != pub_db.end(); it++) {
        printf ("Publisher ID : %u, Publisher Name : %s\n", 
            it->second->publisher_id, it->second->pub_name);
        /* print published messages*/
        for (int i = 0; i < MAX_PUBLISHED_MSG; i++) {
            if (it->second->published_msg_ids[i]) {
                printf ("    Published Message ID : %u\n", it->second->published_msg_ids[i]);
            }
        }
    }

    printf ("Subscriber DB\n");
    for (auto it = sub_db.begin(); it != sub_db.end(); it++) {
        printf ("Subscriber ID : %u, Subscriber Name : %s\n", 
            it->second->subscriber_id, it->second->sub_name);
        /* print subscribed messages*/
        for (int i = 0; i < MAX_SUBSCRIBED_MSG; i++) {
            if (it->second->subscriber_msg_ids[i]) {
                printf ("    Subscribed Message ID : %u\n", it->second->subscriber_msg_ids[i]);
            }
        }
    }

    printf ("Pub-Sub DB\n");
    for (auto it = pub_sub_db.begin(); it != pub_sub_db.end(); it++) {
        printf ("Message ID : %u\n", it->second->publish_msg_code);
        for (auto& sub : it->second->subscribers) {
            printf ("Subscriber ID : %u\n", sub->subscriber_id);
        }
    }
}
