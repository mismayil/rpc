#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <map>
#include "rpc.h"
#include "sock.h"

struct FUNC_SIGNATURE;

/*
* Protocol definitions
*/

// common definitions
#define SIZE_CHAR           sizeof(char)
#define SIZE_SHORT          sizeof(short)
#define SIZE_INT            sizeof(int)
#define SIZE_LONG           sizeof(long)
#define SIZE_FLOAT          sizeof(float)
#define SIZE_DOUBLE         sizeof(double)
#define MAX_FUNC_NAME_LEN   64
#define MAX_SERVER_NAME_LEN 256
#define ARG_TERMINATOR      0
#define NULL_TERMINATOR     '\0'
#define ARG_TYPE_SHIFT      16
#define ARG_TYPE_MASK       0x00ff0000
#define ARG_LEN_MASK        0x0000ffff
#define ARG_IO_MASK         0xff000000

// request types
#define REQUEST_REGISTER  1
#define REQUEST_LOCATION  2
#define REQUEST_EXECUTE   3
#define REQUEST_TERMINATE 4

// return codes
#define RETURN_SUCCESS    0
#define RETURN_FAILURE   -1
#define REGISTER_SUCCESS  5
#define REGISTER_FAILURE -2
#define LOCATION_SUCCESS  6
#define LOCATION_FAILURE -3
#define EXECUTE_SUCCESS   7
#define EXECUTE_FAILURE  -4

// error codes
#define ESOCKET          -5   // error opening socket
#define ENOHOST          -6   // error no host found
#define ECONNECT         -7   // error connecting
#define EUNKNOWN         -8   // unknown error
#define ENOSERVER        -9   // error no available server
#define ETHREAD          -10  // error creating thread
#define EACCEPT          -11  // error accepting connection
#define EBIND            -12  // error binding socket
#define ENOFUNCTION      -13  // error no function found
#define EIBINDER         -14  // error invalid binder
#define ENOLOCATION      -15  // error no location found

// warning codes
#define WDUPREG          10   // warning duplicate registration
#define WNOLOCATION      11   // warning no location found

// rpc general message
class MESSAGE {
protected:
    int len;
    char *buf;
public:
    MESSAGE();
    virtual ~MESSAGE();
    virtual int marshall()=0;
    static MESSAGE* unmarshall(char *msg);
    int getlen();
    char *getbuf();
};

// register request message
class REQ_REG_MESSAGE: public MESSAGE {
public:
    char *serverID;
    int port;
    char *name;
    int *argTypes;
    REQ_REG_MESSAGE(char *serverID, int port, char *name, int *argTypes);
    ~REQ_REG_MESSAGE();
    int marshall() override;
    static MESSAGE* unmarshall(char *msg);
};

// register success response message
class RES_REG_SUCCESS_MESSAGE: public MESSAGE {
public:
    int reasonCode;
    RES_REG_SUCCESS_MESSAGE(int reasonCode);
    int marshall() override;
    static MESSAGE* unmarshall(char *msg);
};

// location request message
class REQ_LOC_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    REQ_LOC_MESSAGE(char *name, int *argTypes);
    ~REQ_LOC_MESSAGE();
    int marshall() override;
    static MESSAGE* unmarshall(char *msg);
};

// location success response message
class RES_LOC_SUCCESS_MESSAGE: public MESSAGE {
public:
    char *serverID;
    int port;
    RES_LOC_SUCCESS_MESSAGE(char *serverID, int port);
    ~RES_LOC_SUCCESS_MESSAGE();
    int marshall() override;
    static MESSAGE* unmarshall(char *msg);
};

// execute request message
class REQ_EXEC_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    void **args;
    REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args);
    ~REQ_EXEC_MESSAGE();
    int marshall() override;
    static MESSAGE* unmarshall(char *msg);
};

// execute success response message
class RES_EXEC_SUCCESS_MESSAGE: public MESSAGE {
public:
    char *name;
    int *argTypes;
    void **args;
    RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args);
    ~RES_EXEC_SUCCESS_MESSAGE();
    int marshall() override;
    static MESSAGE* unmarshall(char *msg);
};

// terminate message
class REQ_TERM_MESSAGE: public MESSAGE {
public:
    int marshall() override;
    static MESSAGE* unmarshall(char *msg);
};

// failure response message
class RES_FAILURE_MESSAGE: public MESSAGE {
public:
    int reasonCode;
    RES_FAILURE_MESSAGE(int reasonCode);
    int marshall() override;
    static MESSAGE* unmarshall(char *msg);
};

// tcp segment
class SEGMENT {
    int len;
    char *buf;
public:
    int length;
    int type;
    MESSAGE *message;
    SEGMENT(int type, MESSAGE *message);
    ~SEGMENT();
    int encapsulate();
    static SEGMENT* decapsulate(int type, char *msg);
    int getlen();
    char *getbuf();
};

// server socket for connections with binder and clients
class SERVER_SOCK: public SOCK {
    std::map<FUNC_SIGNATURE, skeleton> funcmap;
public:
    SERVER_SOCK(int portnum);
    int handle_request(int i);
    int registerFunction(FUNC_SIGNATURE signature, skeleton f);
    int executeFunction(FUNC_SIGNATURE signature, int *argTypes, void **args);
};

// marshalls argTypes and args
int marshallArgs(int *argTypes, void **args, char **buf, int *buflen);

// unmarshalls argTypes and args
int unmarshallArgs(char *msg, int **argTypes, void ***args);

#endif
