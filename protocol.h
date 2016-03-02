#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <string>

/*
* Protocol definitions
*/

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
#define ENOBINDER        -6   // error no binder found
#define ECONNECT         -7   // error connecting

// rpc general message
class MESSAGE {
public:
    virtual char* getBuf()=0;
};

// register request message
class REQ_REG_MESSAGE: public MESSAGE {
public:
    char *server;
    int port;
    char *name;
    int *argTypes;
    REQ_REG_MESSAGE(char *server, int port, char *name, int *argTypes);
    char* getBuf() override;
};

// register success response message
class RES_REG_SUCCESS_MESSAGE: public MESSAGE {
public:
    int reasonCode;
    RES_REG_SUCCESS_MESSAGE(int reasonCode);
    char* getBuf() override;
};

// location request message
class REQ_LOC_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    REQ_LOC_MESSAGE(char *name, int *argTypes);
    char* getBuf() override;
};

// location success response message
class RES_LOC_SUCCESS_MESSAGE: public MESSAGE {
public:
    char *server;
    int port;
    RES_LOC_SUCCESS_MESSAGE(char *server, int port);
    char* getBuf() override;
};

// execute request message
class REQ_EXEC_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    void **args;
    REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args);
    char* getBuf() override;
};

// execute success response message
class RES_EXEC_SUCCESS_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    void **args;
    RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args);
    char* getBuf() override;
};

class REQ_TERM_MESSAGE: public MESSAGE {
public:
    char* getBuf() override;
};

// failure response message
class RES_FAILURE_MESSAGE: public MESSAGE {
public:
    int reasonCode;
    RES_FAILURE_MESSAGE(int reasonCode);
    char* getBuf() override;
};

// tcp segment
class SEGMENT {
public:
    int length;
    int type;
    MESSAGE *message;
    char* getBuf();
    SEGMENT(int length, int type, MESSAGE *message);
};

// error printing function
void error(std::string msg);

#endif
