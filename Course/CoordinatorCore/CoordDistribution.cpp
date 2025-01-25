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


void 
coordinator_fork_distribution_threads() {

} 