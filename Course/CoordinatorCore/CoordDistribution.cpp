#include <unordered_map>
#include <pthread.h>
#include <vector>
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
               // To Do 
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

    }

    vector_data_t *
    Dequeue () {

        
    }
}






void 
coordinator_fork_distribution_threads() {


} 