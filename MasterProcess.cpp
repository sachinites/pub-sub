
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "Common/ipc_struct.h"

extern void coordinator_main();
extern void *pub_skt_example (void *);
extern void *sub_skt_example (void *);
extern void *pub_cbk_example (void *);
extern void *sub_cbk_example (void *);
extern void subscriber_callback (cmsg_t *cmsg ) ;

int 
main (int argc, char **argv) {


    printf ("Master Process : Starting Coordinator Thread\n");
    coordinator_main ();
    getchar();

    printf ("Master Process : Starting UDP Socket Subscriber Thread1\n");
    static pthread_t sub_skt_thread;
    pthread_attr_t sub_skt_thread_attr;
    pthread_attr_init (&sub_skt_thread_attr);
    static ipc_struct_t ipc_struct_sub_skt;
    ipc_struct_sub_skt.netskt.ip_addr =   2130706433 ; //127.0.0.1 
    ipc_struct_sub_skt.netskt.port = 50001;
    pthread_create (&sub_skt_thread, 
        &sub_skt_thread_attr,  
        sub_skt_example, (void *)&ipc_struct_sub_skt);
    getchar();


    printf ("Master Process : Starting UDP Subscriber Thread2\n");
    static pthread_t sub_skt_thread2;
    pthread_attr_t sub_skt_thread_attr2;
    pthread_attr_init (&sub_skt_thread_attr2);
    static ipc_struct_t ipc_struct_sub_skt2;
    ipc_struct_sub_skt2.netskt.ip_addr =   2130706433 ; //127.0.0.1 
     ipc_struct_sub_skt2.netskt.port = 50002;
    pthread_create (&sub_skt_thread2, 
        &sub_skt_thread_attr2,
        sub_skt_example, (void *)&ipc_struct_sub_skt2);    
    getchar();


    printf ("Master Process : Starting CBK Subscriber Thread\n");
    static pthread_t sub_thread_cbk;
    pthread_attr_t sub_thread_cbk_attr;
    pthread_attr_init (&sub_thread_cbk_attr);
    static ipc_struct_t ipc_struct_sub_cbk;
    ipc_struct_sub_cbk.cbk.cbk = subscriber_callback;
    pthread_create (&sub_thread_cbk, 
        &sub_thread_cbk_attr,
        sub_cbk_example, (void *)&ipc_struct_sub_cbk);    
    getchar();


    printf ("Master Process : Starting Skt Publisher Thread\n");
    static pthread_t pub_skt_thread;
    pthread_attr_t pub_skt_attr;
    pthread_attr_init (&pub_skt_attr);
    static ipc_struct_t ipc_struct_pub_skt;
    ipc_struct_pub_skt.netskt.ip_addr =   2130706433 ; //127.0.0.1 
    ipc_struct_pub_skt.netskt.port = 50000;
    pthread_create (&pub_skt_thread, &pub_skt_attr,  pub_skt_example, (void *)&ipc_struct_pub_skt);
    getchar();


    printf ("Master Process : Starting Cbk Publisher Thread\n");
    static pthread_t pub_cbk_thread;
    pthread_attr_t pub_cbk_attr;
    pthread_attr_init (&pub_cbk_attr);
    pthread_create (&pub_cbk_thread, &pub_cbk_attr,  pub_cbk_example, NULL);
    getchar();

    printf ("Master Process started\n");
    
    pthread_exit (0);
    return 0;
}
