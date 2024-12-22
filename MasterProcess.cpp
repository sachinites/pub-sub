
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "Common/ipc_struct.h"

extern void coordinator_main();
extern void *pub_example (void *);
extern void *sub_skt_example (void *);
extern void *sub_cbk_example (void *_ipc_struct) ;
extern void subscriber_callback (cmsg_t *cmsg ) ;

int 
main (int argc, char **argv) {


    printf ("Master Process : Starting Coordinator Thread\n");
    coordinator_main ();
    sleep(1);


    printf ("Master Process : Starting Publishter Thread\n");
    static pthread_t pub_thread1;
    pthread_attr_t attr1;
    pthread_attr_init (&attr1);
    static ipc_struct_t ipc_struct_pub;
    ipc_struct_pub.netskt.ip_addr =   2130706433 ; //127.0.0.1 
    ipc_struct_pub.netskt.port = 50000;
    pthread_create (&pub_thread1, &attr1,  pub_example, (void *)&ipc_struct_pub);
    sleep(1);

    printf ("Master Process : Starting UDP Socket Subscriber Thread1\n");
    static pthread_t sub_thread1;
    pthread_attr_t attr2;
    pthread_attr_init (&attr2);
    static ipc_struct_t ipc_struct_sub1;
    ipc_struct_sub1.netskt.ip_addr =   2130706433 ; //127.0.0.1 
    ipc_struct_sub1.netskt.port = 50001;
    pthread_create (&sub_thread1, &attr2,  sub_skt_example, (void *)&ipc_struct_sub1);
    sleep(1);

    printf ("Master Process : Starting UDP Subscriber Thread2\n");
    static pthread_t sub_thread2;
    pthread_attr_t attr3;
    pthread_attr_init (&attr3);
    static ipc_struct_t ipc_struct_sub2;
    ipc_struct_sub2.netskt.ip_addr =   2130706433 ; //127.0.0.1 
    ipc_struct_sub2.netskt.port = 50002;
    pthread_create (&sub_thread1, &attr3,  sub_skt_example, (void *)&ipc_struct_sub2);    
    sleep(1);

    printf ("Master Process : Starting CBK Subscriber Thread\n");
    static pthread_t sub_thread_cbk;
    pthread_attr_t attr4;
    pthread_attr_init (&attr4);
    static ipc_struct_t ipc_struct_sub_cbk;
    ipc_struct_sub_cbk.cbk.cbk = subscriber_callback;
    pthread_create (&sub_thread_cbk, &attr4,  sub_cbk_example, (void *)&ipc_struct_sub_cbk);    
    sleep(1);

    pthread_exit (0);
    return 0;
}
