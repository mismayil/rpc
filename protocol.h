#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/*
* Protocol definitions
*/

// request types
#define REQ_REGISTER  0
#define REQ_LOC       1
#define REQ_EXECUTE   2
#define REQ_TERMINATE 3

// return types
#define RETURN_SUCCESS    0
#define RETURN_FAILURE   -1
#define REGISTER_SUCCESS  0
#define REGISTER_FAILURE -2
#define LOC_SUCCESS       0
#define LOC_FAILURE      -3
#define EXECUTE_SUCCESS   0
#define EXECUTE_FAILURE  -4

#endif
