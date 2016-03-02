#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "rpc.h"

/*
* Protocol definitions
*/

// common definitions
#define SIZE_CHAR         1
#define SIZE_SHORT        2
#define SIZE_INT          4
#define SIZE_LONG         8
#define SIZE_FLOAT        4
#define SIZE_DOUBLE       8
#define MAX_NAME_LENGTH   64
#define ARG_TERMINATOR    0
#define NULL_TERMINATOR   '\0'
#define ARG_TYPE_SHIFT    16
#define ARG_TYPE_MASK     0x00ff0000
#define ARG_LEN_MASK      0x0000ffff

// request types
#define REQUEST_REGISTER  1
#define REQUEST_LOCATION  2
#define REQUEST_EXECUTE   3
#define REQUEST_TERMINATE 4

// return codes
#define RETURN_SUCCESS    0
#define RETURN_FAILURE   -1
#define REGISTER_SUCCESS  0
#define REGISTER_FAILURE -2
#define LOCATION_SUCCESS  0
#define LOCATION_FAILURE -3
#define EXECUTE_SUCCESS   0
#define EXECUTE_FAILURE  -4

// error codes
#define ESOCKET          -5   // error opening socket
#define ENOHOST          -6   // error no host found
#define ECONNECT         -7   // error connecting
#define EUNKNOWN         -8   // unknown error
#define ENOSERVER        -9   // error no available server

// rpc general message
class MESSAGE {
protected:
    char *buf;
public:
    virtual ~MESSAGE();
};

class REQ_MESSAGE: public MESSAGE {
public:
    virtual char* marshall()=0;
};

class RES_MESSAGE: public MESSAGE {
public:
    virtual void unmarshall()=0;
};

// register request message
class REQ_REG_MESSAGE: public REQ_MESSAGE {
public:
    char *serverID;
    int port;
    char *name;
    int *argTypes;
    REQ_REG_MESSAGE(char *serverID, int port, char *name, int *argTypes);
    char* marshall() override;
};

// register success response message
class RES_REG_SUCCESS_MESSAGE: public RES_MESSAGE {
public:
    int reasonCode;
    RES_REG_SUCCESS_MESSAGE(int reasonCode);
    void unmarshall() override;
};

// location request message
class REQ_LOC_MESSAGE: public REQ_MESSAGE {
public:
    char *name;
    int *argTypes;
    REQ_LOC_MESSAGE(char *name, int *argTypes);
    char* marshall() override;
};

// location success response message
class RES_LOC_SUCCESS_MESSAGE: public RES_MESSAGE {
public:
    char *serverID;
    int port;
    RES_LOC_SUCCESS_MESSAGE(char *serverID, int port);
    void unmarshall() override;
};

// execute request message
class REQ_EXEC_MESSAGE: public REQ_MESSAGE {
public:
    char *name;
    int *argTypes;
    void **args;
    REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args);
    char* marshall() override;
};

// execute success response message
class RES_EXEC_SUCCESS_MESSAGE: public RES_MESSAGE {
public:
    char *name;
    int *argTypes;
    void **args;
    RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args);
    void unmarshall() override;
};

// terminate message
class REQ_TERM_MESSAGE: public REQ_MESSAGE {
public:
    char* marshall() override;
};

// failure response message
class RES_FAILURE_MESSAGE: public RES_MESSAGE {
public:
    int reasonCode;
    RES_FAILURE_MESSAGE(int reasonCode);
    void unmarshall() override;
};

// tcp segment
class SEGMENT {
    char *buf;
public:
    int length;
    int type;
    MESSAGE *message;
    SEGMENT(int type, MESSAGE *message);
    ~SEGMENT();
    char* encapsulate();
    void decapsulate();
};

// error printing function
void error(char *msg);

// copies data to the buffer
void copy(char *buf, void *v, int len=1, int type=ARG_CHAR);

// connects to the host with given address and port
int connectTo(char *address, int port);

// sends a tcp segment
int sendSegment(int sock_fd, SEGMENT *segment);

// receives a tcp segment
SEGMENT* recvSegment(int sock_fd);

#endif
