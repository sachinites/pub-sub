#include <unordered_map>
#include <pthread.h>
#include <vector>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include "CoordDb.h"
#include "pubsub.h"
#include "../Common/comm-types.h"
#include "../Common/cmsgOp.h"

/* Now Distribute the message to interested subscribers */

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
                    free (vdata->cmsg);
                    vdata->sub_entry = NULL;
                    free (vdata);
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
        dist_queue[vdata->cmsg->priority].push_back (vdata);
        pthread_cond_signal (&dist_queue_cond);
        pthread_mutex_unlock (&dist_queue_lock);
    }

    vector_data_t* 
    Dequeue () {

        vector_data_t *vdata = NULL;

        pthread_mutex_lock (&dist_queue_lock);

        while (1) {

            for (int i = 0; i < CMSG_PR_MAX; i++) {
                if (!dist_queue[i].empty()) {
                    vdata = dist_queue[i].front();
                    dist_queue[i].erase (dist_queue[i].begin());
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

    vector_data_t *vdata = NULL;
    CoordDistQueue *dist_queue =  static_cast<CoordDistQueue *>(arg);

    while ((vdata = dist_queue->Dequeue())) {
        coordinator_dispatch (vdata->sub_entry, vdata->cmsg);
        free(vdata->cmsg);
        vdata->sub_entry = NULL;  
        free (vdata);
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
coordinator_enqueue_distribution_queue (
        cmsg_t *cmsg, 
        std::shared_ptr<subscriber_db_entry_t> SubEntry) {

    static int queue_index = 0;

    if (queue_index >= COORD_DIST_QUEUES_MAX) {
        queue_index = 0;
    }

    vector_data_t *vdata = (vector_data_t *)calloc (1, sizeof (vector_data_t));
    vdata->cmsg = cmsg;
    vdata->sub_entry = SubEntry;
    gdist_queue[queue_index]->Enqueue (vdata);
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
        coordinator_enqueue_distribution_queue (dist_msg, sub_entry);
    }
}

/* Finally dispatch the cmsg to Subscriber */
void
coordinator_dispatch (std::shared_ptr<subscriber_db_entry_t> SubEntry, cmsg_t *cmsg) {

    printf ("Coordinator : Dispatching message to subscriber\n");
    cmsg_debug_print (cmsg);

    /* Now send the message to subscriber */
    if (SubEntry->ipc_type == IPC_TYPE_NONE) {
        printf ("Coordinator : Error : Subscriber [%s, %u] IPC Channel Not Set\n", SubEntry->sub_name, SubEntry->subscriber_id);
        return;
    }

    switch (SubEntry->ipc_type) {

        case IPC_TYPE_NETSKT:
        {
            printf ("Coordinator : Dispatching message to subscriber [%s, %u] over NETSKT\n",   
                SubEntry->sub_name, SubEntry->subscriber_id);
            uint32_t ip_addr = SubEntry->ipc_struct.netskt.ip_addr;
            uint16_t port = SubEntry->ipc_struct.netskt.port;
            uint8_t transport_type = SubEntry->ipc_struct.netskt.transport_type;
            
            if (transport_type != IPPROTO_UDP) {
                printf ("Coordinator : Error : Unsupported Transport Type %u\n", transport_type);
                return;
            }

            struct sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(port);
            server_addr.sin_addr.s_addr = htonl(ip_addr);

            if (SubEntry->ipc_struct.netskt.sock_fd > 0) {
                
                int rc = sendto (SubEntry->ipc_struct.netskt.sock_fd, 
                    (char *)cmsg, sizeof (*cmsg) + cmsg->msg_size, 0, 
                    (struct sockaddr *)&server_addr, sizeof (struct sockaddr));

                if (rc < 0) {
                    printf ("Coordinator : Error : Send Failed, errno = %d\n", errno);
                }

                free(cmsg);
                break;
            }

            /* Now send the message to subscriber */
            int sock_fd = socket (AF_INET, SOCK_DGRAM, transport_type);

            if (sock_fd < 0) {
                printf ("Coordinator : Error : Socket Creation Failed\n");
                return;
            }

            SubEntry->ipc_struct.netskt.sock_fd = sock_fd;

            int rc = sendto (sock_fd, (char *)cmsg, sizeof (*cmsg) + cmsg->msg_size, 0, 
                        (struct sockaddr *)&server_addr, sizeof (struct sockaddr));

            if (rc < 0) {
                printf ("Coordinator : Error : Send Failed, errno = %d\n", errno);
            }

            free(cmsg);
        }
        break;

        
        case IPC_TYPE_MSGQ:
        {
            printf ("Coordinator : Dispatching message to subscriber [%s, %u] over MQUEUE\n", SubEntry->sub_name, SubEntry->subscriber_id);
            break;
        }        
        case IPC_TYPE_UXSKT:
        {
            printf ("Coordinator : Dispatching message to subscriber [%s, %u] over UXSKT\n", SubEntry->sub_name, SubEntry->subscriber_id);
            break;
        }        
        case IPC_TYPE_SHM:
        {
            printf ("Coordinator : Dispatching message to subscriber [%s, %u] over SHM\n", SubEntry->sub_name, SubEntry->subscriber_id);
            break;
        }
        case IPC_TYPE_CBK:
        {
            printf ("Coordinator : Dispatching message to subscriber [%s, %u] over CBK\n", SubEntry->sub_name, SubEntry->subscriber_id);
            break;
        }
        default:
            printf ("Coordinator : Error : Unknown IPC Type %u\n", SubEntry->ipc_type);
    }
}