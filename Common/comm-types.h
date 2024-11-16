#ifndef __COMM_TYPES__
#define __COMM_TYPES__

typedef enum msg_type_ {

    SUBS_TO_COORD_MSG,
    COORD_TO_SUBS, 
    PUB_TO_COORD,
    COORD_TO_PUB

}  msg_type_t;

#define COORD_MSGQ_NAME "/MAIN-COORD-MSGQ"

#endif 