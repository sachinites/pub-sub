#ifndef __CLIENT_COMMON_H__
#define __CLIENT_COMMON_H__

/* This file contains fns to be used by clients but those functions directly calls the
    coordinator fns also. */

typedef struct cmsg_  cmsg_t;

void
pub_sub_send_ips (cmsg_t *cmsg) ;

#endif // __CLIENT_COMMON_H__