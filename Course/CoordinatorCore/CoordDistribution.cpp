#include <unordered_map>
#include <pthread.h>
#include <vector>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "coordDb.h"
#include "pubsub.h"
#include "../Common/comm-types.h"
#include "../Common/cmsgOp.h"

typedef struct vector_data_ {

    cmsg_t *cmsg;
    std::shared_ptr<subscriber_db_entry_t> sub_entry;

} vector_data_t;


class CoordDistQueue {

    private:
        std::vector <vector_data_t *> dist_queue[CMSG_PR_MAX];
        pthread_mutex_t dist_queue_lock;
        pthread_cond_t dist_queue_cond;

     void release_distribution_queue() {
        
            for (int i = 0; i < CMSG_PR_MAX; i++) {
              
              for (auto vdata : dist_queue[i]) {
                cmsg_dereference (vdata->cmsg);
                vdata->sub_entry = nullptr;
                free(vdata);
              }
              dist_queue[i].clear();
            }
        }

    public:
            CoordDistQueue () {
            pthread_mutex_init (&dist_queue_lock, NULL);
            pthread_cond_init (&dist_queue_cond, NULL);
        }

    ~CoordDistQueue () {
        pthread_mutex_destroy (&dist_queue_lock);
        pthread_cond_destroy (&dist_queue_cond);
        release_distribution_queue();
    }

    void 
    Enqueue (vector_data_t *vdata) {

        pthread_mutex_lock (&dist_queue_lock);

        dist_queue[vdata->cmsg->priority].push_back(vdata);

        pthread_cond_signal (&dist_queue_cond);

        pthread_mutex_unlock (&dist_queue_lock);
    }

    vector_data_t *
    Dequeue () {

        vector_data_t *vdata = NULL;

        pthread_mutex_lock (&dist_queue_lock);

        while (1) {

            for (int i = 0; i < CMSG_PR_MAX; i++) {

                if (!dist_queue[i].empty()) {
                    vdata = dist_queue[i].front();
                    dist_queue[i].erase(dist_queue[i].begin());
                    break;
                }
            }

            if (vdata) break;
            pthread_cond_wait (&dist_queue_cond, &dist_queue_lock);
        }

        pthread_mutex_unlock (&dist_queue_lock);
        return vdata;
    }

};

#define COORD_DIST_QUEUES_MAX 2
static class CoordDistQueue *gdist_queue[COORD_DIST_QUEUES_MAX] = {0};

static void
coordinator_dispatch (std::shared_ptr<subscriber_db_entry_t> SubEntry, cmsg_t *cmsg) ;

static void *
coordinator_listen_distribution_queue (void *arg) {

    vector_data_t *vdata;
    CoordDistQueue *dist_queue = static_cast<CoordDistQueue *>(arg);

    while ((vdata = dist_queue->Dequeue())) {

        coordinator_dispatch (vdata->sub_entry, vdata->cmsg);
        cmsg_dereference (vdata->cmsg);
        vdata->sub_entry = nullptr;
        free(vdata);
    }

    return NULL;
}


void 
coordinator_fork_distribution_threads() {

    pthread_t *thread;

    for (int i = 0 ; i < COORD_DIST_QUEUES_MAX; i++) {

        if (gdist_queue[i] == NULL) {

            gdist_queue[i] = new CoordDistQueue();
            thread = (pthread_t *)calloc (1, sizeof (pthread_t ));
            pthread_create (thread, NULL, coordinator_listen_distribution_queue, (void *)gdist_queue[i]);
        }
    }
} 

/* USe Round Robin method for load balancing on Distribution Queues*/
static void 
coordinator_enqueue_distribution_queue (
        cmsg_t *cmsg, 
        std::shared_ptr<subscriber_db_entry_t> SubEntry) {

    static int queue_index = 0;

    if (queue_index >= COORD_DIST_QUEUES_MAX) {
        queue_index = 0;
    }

     vector_data_t *vdata = (vector_data_t *)calloc (1, sizeof (vector_data_t));

     vdata->cmsg = cmsg;
     cmsg_reference (cmsg);
     vdata->sub_entry = SubEntry;

     gdist_queue[queue_index]->Enqueue(vdata);
     queue_index++;

    printf ("Coordinator : [cmsg, Sub : %s] Enqueued in Distribution Queue\n", SubEntry->sub_name);
}

void 
coordinator_accept_pubmsg_for_distribution_to_subcribers (cmsg_t *cmsg)  {

    cmsg->msg_type = COORD_TO_SUBS;

    pub_sub_db_entry_t *pub_sub_entry = pub_sub_db_get (cmsg->msg_code);

    if (!pub_sub_entry) {

        return;
    }

    for (auto sub_entry : pub_sub_entry->subscribers) {

        coordinator_enqueue_distribution_queue (cmsg, sub_entry);
    }
}

void
coordinator_dispatch (std::shared_ptr<subscriber_db_entry_t> SubEntry, cmsg_t *cmsg)  {

    printf ("Distributing cmsg %u to Subscriber %s[%u]\n", 
        cmsg->msg_id, SubEntry->sub_name, SubEntry->subscriber_id);
}