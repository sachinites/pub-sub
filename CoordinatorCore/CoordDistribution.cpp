#include <unordered_map>
#include <pthread.h>
#include <vector>
#include "CoordDb.h"
#include "pubsub.h"
#include "../Common/comm-types.h"
#include "../Common/cmsgOp.h"

/* Now Distribute the message to interested subscribers */

class CoordDistQueue {

    private:
        std::vector <cmsg_t *> dist_queue[CMSG_PR_MAX];
        pthread_mutex_t dist_queue_lock;
        pthread_cond_t dist_queue_cond;

        void release_distribution_queue() {
            for (int i = 0; i < CMSG_PR_MAX; i++) {
                for (auto msg : dist_queue[i]) {
                    free (msg);
                }
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
    void Enqueue (cmsg_t *msg) {
        pthread_mutex_lock (&dist_queue_lock);
        dist_queue[msg->priority].push_back (msg);
        pthread_cond_signal (&dist_queue_cond);
        pthread_mutex_unlock (&dist_queue_lock);
    }
    cmsg_t *Dequeue () {
        cmsg_t *msg = NULL;
        pthread_mutex_lock (&dist_queue_lock);
        while (1) {
            for (int i = 0; i < CMSG_PR_MAX; i++) {
                if (!dist_queue[i].empty()) {
                    msg = dist_queue[i].front();
                    dist_queue[i].erase (dist_queue[i].begin());
                    break;
                }
            }
            if (msg) break;
            pthread_cond_wait (&dist_queue_cond, &dist_queue_lock);
        }
        pthread_mutex_unlock (&dist_queue_lock);
        return msg;
    }
};

#define COORD_DIST_QUEUES_MAX 2
static class CoordDistQueue *gdist_queue[COORD_DIST_QUEUES_MAX] = {0};

static void
coordinator_dispatch (cmsg_t *cmsg) ;

static void *
coordinator_listen_distribution_queue (void *arg) {

    CoordDistQueue *dist_queue = (CoordDistQueue *)arg;
    cmsg_t *cmsg = NULL;

    while ((cmsg = dist_queue->Dequeue())) {

        coordinator_dispatch (cmsg);
        free(cmsg);
    }

    return NULL;
}

void
coordinator_fork_distribution_threads() {

    pthread_t *thread;
    for (int i = 0; i < COORD_DIST_QUEUES_MAX; i++) {
        if (gdist_queue[i] == NULL) {
            gdist_queue[i] = new CoordDistQueue();
            thread = (pthread_t *)calloc (1, sizeof (pthread_t));
            pthread_create (thread, NULL, coordinator_listen_distribution_queue, (void *)gdist_queue[i]);
        }
    }
}

/* USe Round Robin method for load balancing on Distribution Queues*/
static void 
coordinator_enqueue_distribution_queue (cmsg_t *cmsg) {

    static int queue_index = 0;

    if (queue_index >= COORD_DIST_QUEUES_MAX) {
        queue_index = 0;
    }

    gdist_queue[queue_index]->Enqueue (cmsg);
    queue_index++;
}


void 
coordinator_accept_pubmsg_for_distribution_to_subcribers (cmsg_t *cmsg) {
    
    /* Get the subscribers interested in this message */
    pub_sub_db_entry_t *pub_sub_entry = pub_sub_db_get (cmsg->msg_code);

    if (!pub_sub_entry) {
        return;
    }

    /* Distribute the message to interested subscribers */
    for (auto sub_entry : pub_sub_entry->subscribers) {
        cmsg_t *dist_msg = (cmsg_t *)  calloc   (1, sizeof (*dist_msg) + cmsg->msg_size - cmsg->tlv_buffer_size);
        memcpy (dist_msg, cmsg, sizeof (cmsg_t) );
        /* Now override the fields as required*/
        dist_msg->msg_type = COORD_TO_SUBS;
        dist_msg->tlv_buffer_size = 0;
        dist_msg->msg_size = cmsg->msg_size - cmsg->tlv_buffer_size;
        memcpy (dist_msg->msg, (char *)cmsg->msg + cmsg->tlv_buffer_size, dist_msg->msg_size);
        coordinator_enqueue_distribution_queue (dist_msg);
    }
}

/* Finally dispatch the cmsg to Subscriber */
void
coordinator_dispatch (cmsg_t *cmsg) {

    printf ("Coordinator : Dispatching message to subscriber\n");
    cmsg_debug_print (cmsg);
    
}