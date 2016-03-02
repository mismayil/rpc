#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

/*
* Protocol definitions
*/

// common definitions
#define MAX_NAME_LENGTH   64
#define INT_SIZE          4

// request types
#define REQUEST_REGISTER  0
#define REQUEST_LOCATION  1
#define REQUEST_EXECUTE   2
#define REQUEST_TERMINATE 3

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
    virtual char* getbuf();
    ~MESSAGE();
};

// register request message
class REQ_REG_MESSAGE: public MESSAGE {
public:
    char *serverID;
    int port;
    char *name;
    int *argTypes;
    REQ_REG_MESSAGE(char *serverID, int port, char *name, int *argTypes);
};

// register success response message
class RES_REG_SUCCESS_MESSAGE: public MESSAGE {
public:
    int reasonCode;
    RES_REG_SUCCESS_MESSAGE(int reasonCode);
};

// location request message
class REQ_LOC_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    REQ_LOC_MESSAGE(char *name, int *argTypes);
};

// location success response message
class RES_LOC_SUCCESS_MESSAGE: public MESSAGE {
public:
    char *serverID;
    int port;
    RES_LOC_SUCCESS_MESSAGE(char *serverID, int port);
};

// execute request message
class REQ_EXEC_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    void **args;
    REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args);
};

// execute success response message
class RES_EXEC_SUCCESS_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    void **args;
    RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args);
};

// terminate message
class REQ_TERM_MESSAGE: public MESSAGE {};

// failure response message
class RES_FAILURE_MESSAGE: public MESSAGE {
public:
    int reasonCode;
    RES_FAILURE_MESSAGE(int reasonCode);
};

// tcp segment
class SEGMENT {
    char *buf;
public:
    int length;
    int type;
    MESSAGE *message;
    char* getbuf();
    SEGMENT(int type, MESSAGE *message);
    ~SEGMENT();
};

// error printing function
void error(char *msg);
int buflen(char *buf);
void copy_int_to_buf(int n, char *buf);
int connectTo(char *address, int port);
int sendSegment(int sock_fd, SEGMENT *segment);
SEGMENT* recvSegment(int sock_fd);

#endif
