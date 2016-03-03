#include <iostream>
#include <vector>
#include <cstring>
#include "util.h"
#include "protocol.h"

using namespace std;

SEGMENT::SEGMENT(int type, MESSAGE *message) : type(type), message(message) {}

SEGMENT::~SEGMENT() {
    if (buf) delete [] buf;
}

char* SEGMENT::encapsulate() {
    char *msgbuf = message->marshall();
    length = SIZE_INT * 2 + strlen(msgbuf) + 1;
    buf = new char[length];
    char *bufptr = buf;

    copy(bufptr, (void *) &length, 1, ARG_INT);
    bufptr += SIZE_INT;

    copy(bufptr, (void *) &type, 1, ARG_INT);
    bufptr += SIZE_INT;

    copy(bufptr, (void *) msgbuf, strlen(msgbuf));

    buf[length-1] = NULL_TERMINATOR;

    return buf;
}

SEGMENT* SEGMENT::decapsulate(int type, char *msg) {
    MESSAGE *message;

    switch (type) {
        case REQUEST_REGISTER:
            message = REQ_REG_MESSAGE::unmarshall(msg);
            break;
        case REQUEST_LOCATION:
            message = REQ_LOC_MESSAGE::unmarshall(msg);
            break;
        case REQUEST_EXECUTE:
            message = REQ_EXEC_MESSAGE::unmarshall(msg);
            break;
        case REQUEST_TERMINATE:
            message = REQ_TERM_MESSAGE::unmarshall(msg);
            break;
        case REGISTER_SUCCESS:
            message = RES_REG_SUCCESS_MESSAGE::unmarshall(msg);
            break;
        case REGISTER_FAILURE:
            message = RES_FAILURE_MESSAGE::unmarshall(msg);
            break;
        case LOCATION_SUCCESS:
            message = RES_LOC_SUCCESS_MESSAGE::unmarshall(msg);
            break;
        case LOCATION_FAILURE:
            message = RES_FAILURE_MESSAGE::unmarshall(msg);
            break;
        case EXECUTE_SUCCESS:
            message = RES_EXEC_SUCCESS_MESSAGE::unmarshall(msg);
            break;
        case EXECUTE_FAILURE:
            message = RES_FAILURE_MESSAGE::unmarshall(msg);
            break;
    }

    SEGMENT *segment = new SEGMENT(type, message);

    return segment;
}

MESSAGE::~MESSAGE() {
    if (buf) delete [] buf;
}

MESSAGE* MESSAGE::unmarshall(char *msg) { return NULL; }

REQ_REG_MESSAGE::REQ_REG_MESSAGE(char *serverID, int port, char *name, int *argTypes) : serverID(serverID), port(port), name(name), argTypes(argTypes) {}

REQ_REG_MESSAGE::~REQ_REG_MESSAGE() {
    if (serverID) delete [] serverID;
    if (name) delete [] name;
    if (argTypes) delete [] argTypes;
}

char* REQ_REG_MESSAGE::marshall() {
    int argc = 0;

    if (argTypes != NULL) {
        while (argTypes[argc] != ARG_TERMINATOR) {
            argc++;
        }
    }

    int len = strlen(serverID) + 1 + SIZE_INT + strlen(name) + 1 + argc * SIZE_INT + 1;
    buf = new char[len];
    char *bufptr = buf;

    // marshall serverID
    copy(bufptr, (void *) serverID, strlen(serverID));
    bufptr += strlen(serverID);
    *bufptr = NULL_TERMINATOR;
    bufptr++;

    // marshall port
    copy(bufptr, (void *) &port, 1, ARG_INT);
    bufptr += SIZE_INT;

    // marshall name
    copy(bufptr, (void *) name, strlen(name));
    bufptr += strlen(name);
    *bufptr = NULL_TERMINATOR;
    bufptr++;

    // marshall argTypes
    copy(bufptr, (void *) argTypes, argc, ARG_INT);

    buf[len-1] = NULL_TERMINATOR;

    return buf;
}

MESSAGE* REQ_REG_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    int len = 0;
    char intbuf[SIZE_INT];
    char *serverID;
    int port, argType;
    char *name;
    int *argTypes;
    vector<int> types;

    // unmarshall server identifier
    while (*(msgptr + len) != NULL_TERMINATOR) {
        len++;
    }

    serverID = new char[len];
    copy(serverID, (void *) msgptr, len);
    serverID[len-1] = NULL_TERMINATOR;
    msgptr += len;

    // unmarshall port
    copy(intbuf, (void *) msgptr, SIZE_INT);
    port = ctoi(intbuf);
    msgptr += SIZE_INT;

    // unmarshall name
    len = 0;
    while (*(msgptr + len) != NULL_TERMINATOR) {
        len++;
    }

    name = new char[len];
    copy(name, (void *) msgptr, len);
    name[len-1] = NULL_TERMINATOR;
    msgptr += len;

    // unmarshall arg types
    while (1) {
        copy(intbuf, (void *) msgptr, SIZE_INT);
        msgptr += SIZE_INT;
        argType = ctoi(intbuf);
        types.push_back(argType);
        if (argType == ARG_TERMINATOR) break;
    }

    argTypes = new int[types.size()];

    for (unsigned int i = 0; i < types.size(); i++) {
        argTypes[i] = types[i];
    }

    MESSAGE* message = new REQ_REG_MESSAGE(serverID, port, name, argTypes);

    return message;
}

RES_REG_SUCCESS_MESSAGE::RES_REG_SUCCESS_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

char* RES_REG_SUCCESS_MESSAGE::marshall() {
    // marshall reasonCode
    buf = new char[SIZE_INT];
    copy(buf, (void *) &reasonCode, 1, ARG_INT);
    return buf;
}

MESSAGE* RES_REG_SUCCESS_MESSAGE::unmarshall(char *msg) {
    // unmarshall reasonCode
    int reasonCode;
    reasonCode = ctoi(msg);
    MESSAGE *message = new RES_REG_SUCCESS_MESSAGE(reasonCode);
    return message;
}

REQ_LOC_MESSAGE::REQ_LOC_MESSAGE(char *name, int *argTypes) : name(name), argTypes(argTypes) {}

REQ_LOC_MESSAGE::~REQ_LOC_MESSAGE() {
    if (name) delete [] name;
    if (argTypes) delete [] argTypes;
}

char* REQ_LOC_MESSAGE::marshall() {
    int argc = 0;

    if (argTypes != NULL) {
        while (argTypes[argc] != ARG_TERMINATOR) {
            argc++;
        }
    }

    int len = strlen(name) + 1 + argc * SIZE_INT + 1;
    buf = new char[len];
    char *bufptr = buf;

    // marshall name
    copy(bufptr, (void *) name, strlen(name));
    bufptr += strlen(name);
    *bufptr = NULL_TERMINATOR;
    bufptr++;

    // marshall argTypes
    copy(bufptr, (void *) argTypes, argc, ARG_INT);

    buf[len-1] = NULL_TERMINATOR;

    return buf;
}

MESSAGE* REQ_LOC_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    int len = 0;
    char intbuf[SIZE_INT];
    int argType;
    char *name;
    int *argTypes;
    vector<int> types;

    // unmarshall name
    while (*(msgptr + len) != NULL_TERMINATOR) {
        len++;
    }

    name = new char[len];
    copy(name, (void *) msgptr, len);
    name[len-1] = NULL_TERMINATOR;
    msgptr += len;

    // unmarshall arg types
    while (1) {
        copy(intbuf, (void *) msgptr, SIZE_INT);
        msgptr += SIZE_INT;
        argType = ctoi(intbuf);
        types.push_back(argType);
        if (argType == ARG_TERMINATOR) break;
    }

    argTypes = new int[types.size()];

    for (unsigned int i = 0; i < types.size(); i++) {
        argTypes[i] = types[i];
    }

    MESSAGE* message = new REQ_LOC_MESSAGE(name, argTypes);

    return message;
}

RES_LOC_SUCCESS_MESSAGE::RES_LOC_SUCCESS_MESSAGE(char *serverID, int port) : serverID(serverID), port(port) {}

RES_LOC_SUCCESS_MESSAGE::~RES_LOC_SUCCESS_MESSAGE() {
    if (serverID) delete [] serverID;
}

char* RES_LOC_SUCCESS_MESSAGE::marshall() {
    int len = strlen(serverID) + 1 + SIZE_INT + 1;
    buf = new char[len];
    char *bufptr = buf;

    // marshall serverID
    copy(bufptr, (void *) serverID, strlen(serverID));
    bufptr += strlen(serverID);
    *bufptr = NULL_TERMINATOR;
    bufptr++;

    // marshall port
    copy(bufptr, (void *) &port, 1, ARG_INT);

    buf[len-1] = NULL_TERMINATOR;

    return buf;
}

MESSAGE* RES_LOC_SUCCESS_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    int len = 0;
    char intbuf[SIZE_INT];
    char *serverID;
    int port;

    // unmarshall server identifier
    while (*(msgptr + len) != NULL_TERMINATOR) {
        len++;
    }

    serverID = new char[len];
    copy(serverID, (void *) msgptr, len);
    serverID[len-1] = NULL_TERMINATOR;
    msgptr += len;

    // unmarshall port
    copy(intbuf, (void *) msgptr, SIZE_INT);
    port = ctoi(intbuf);

    MESSAGE* message = new RES_LOC_SUCCESS_MESSAGE(serverID, port);

    return message;
}

REQ_EXEC_MESSAGE::REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

REQ_EXEC_MESSAGE::~REQ_EXEC_MESSAGE() {
    if (name) delete [] name;
    if (argTypes) delete [] argTypes;
    // delete args, how?
}

char* REQ_EXEC_MESSAGE::marshall() {
    int len = strlen(name) + 1;

    // marshall argTypes and args
    char *argbuf = marshallArgs(argTypes, args);

    len += strlen(argbuf) + 1;
    buf = new char[len];
    char *bufptr = buf;

    // marshall name
    copy(bufptr, (void *) name, strlen(name));
    bufptr += strlen(name);
    *bufptr = NULL_TERMINATOR;
    bufptr++;

    copy(bufptr, (void *) argbuf, strlen(argbuf));

    buf[len-1] = NULL_TERMINATOR;

    return buf;
}

MESSAGE* REQ_EXEC_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    int len = 0;
    char *name;
    int *argTypes = NULL;
    void **args = NULL;

    // unmarshall name
    while (*(msgptr + len) != NULL_TERMINATOR) {
        len++;
    }

    name = new char[len];
    copy(name, (void *) msgptr, len);
    name[len-1] = NULL_TERMINATOR;
    msgptr += len;

    // unmarshall arg types and args
    unmarshallArgs(msgptr, argTypes, args);

    MESSAGE* message = new REQ_EXEC_MESSAGE(name, argTypes, args);

    return message;
}

char* REQ_TERM_MESSAGE::marshall() {
    buf = new char[1];
    buf[0] = NULL_TERMINATOR;
    return buf;
}

MESSAGE* REQ_TERM_MESSAGE::unmarshall(char *msg) {
    return new REQ_TERM_MESSAGE();
}

RES_EXEC_SUCCESS_MESSAGE::RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

RES_EXEC_SUCCESS_MESSAGE::~RES_EXEC_SUCCESS_MESSAGE() {
    if (name) delete [] name;
    if (argTypes) delete [] argTypes;
    // delete args, how?
}

char* RES_EXEC_SUCCESS_MESSAGE::marshall() {
    int len = strlen(name) + 1;

    // marshall argTypes and args
    char *argbuf = marshallArgs(argTypes, args);

    len += strlen(argbuf) + 1;
    buf = new char[len];
    char *bufptr = buf;

    // marshall name
    copy(bufptr, (void *) name, strlen(name));
    bufptr += strlen(name);
    *bufptr = NULL_TERMINATOR;
    bufptr++;

    copy(bufptr, (void *) argbuf, strlen(argbuf));

    buf[len-1] = NULL_TERMINATOR;

    return buf;
}

MESSAGE* RES_EXEC_SUCCESS_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    int len = 0;
    char *name;
    int *argTypes = NULL;
    void **args = NULL;

    // unmarshall name
    while (*(msgptr + len) != NULL_TERMINATOR) {
        len++;
    }

    name = new char[len];
    copy(name, (void *) msgptr, len);
    name[len-1] = NULL_TERMINATOR;
    msgptr += len;

    // unmarshall arg types and args
    unmarshallArgs(msgptr, argTypes, args);

    MESSAGE* message = new RES_EXEC_SUCCESS_MESSAGE(name, argTypes, args);

    return message;
}

RES_FAILURE_MESSAGE::RES_FAILURE_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

char* RES_FAILURE_MESSAGE::marshall() {
    // marshall reasonCode
    buf = new char[SIZE_INT];
    copy(buf, (void *) &reasonCode, 1, ARG_INT);
    return buf;
}

MESSAGE* RES_FAILURE_MESSAGE::unmarshall(char *msg) {
    // unmarshall reasonCode
    int reasonCode;
    reasonCode = ctoi(msg);
    MESSAGE *message = new RES_FAILURE_MESSAGE(reasonCode);
    return message;
}

char* marshallArgs(int *argTypes, void **args) {
    int arglen, type, argType, argc = 0;
    char *buf;

    if (argTypes != NULL) {
        while (argTypes[argc] != ARG_TERMINATOR) {
            argc++;
        }
    }

    int len = argc * SIZE_INT;

    for (int i = 0; i < argc - 1; i++) {
        argType = argTypes[i];
        type = (argType & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        arglen = argType & ARG_LEN_MASK;
        arglen = arglen > 0 ? arglen : 1;

        switch (type) {
            case ARG_CHAR:
                len += arglen * SIZE_CHAR;
                break;
            case ARG_SHORT:
                len += arglen * SIZE_SHORT;
                break;
            case ARG_INT:
                len += arglen * SIZE_INT;
                break;
            case ARG_LONG:
                len += arglen * SIZE_LONG;
                break;
            case ARG_FLOAT:
                len += arglen * SIZE_FLOAT;
                break;
            case ARG_DOUBLE:
                len += arglen * SIZE_DOUBLE;
                break;
        }
    }

    buf = new char[len+1];
    char *bufptr = buf;
    copy(bufptr, (void *) argTypes, argc, ARG_INT);
    bufptr += argc * SIZE_INT;

    for (int i = 0; i < argc - 1; i++) {
        argType = argTypes[i];
        type = (argType & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        arglen = argType & ARG_LEN_MASK;
        arglen = arglen > 0 ? arglen : 1;

        switch (type) {
            case ARG_CHAR:
                copy(bufptr, (void *) args[i], arglen, ARG_CHAR);
                bufptr += arglen * SIZE_CHAR;
                break;
            case ARG_SHORT:
                copy(bufptr, (void *) args[i], arglen, ARG_SHORT);
                bufptr += arglen * SIZE_SHORT;
                break;
            case ARG_INT:
                copy(bufptr, (void *) args[i], arglen, ARG_INT);
                bufptr += arglen * SIZE_INT;
                break;
            case ARG_LONG:
                copy(bufptr, (void *) args[i], arglen, ARG_LONG);
                bufptr += arglen * SIZE_LONG;
                break;
            case ARG_FLOAT:
                copy(bufptr, (void *) args[i], arglen, ARG_FLOAT);
                bufptr += arglen * SIZE_FLOAT;
                break;
            case ARG_DOUBLE:
                copy(bufptr, (void *) args[i], arglen, ARG_DOUBLE);
                bufptr += arglen * SIZE_DOUBLE;
                break;
        }
    }

    buf[len-1] = NULL_TERMINATOR;

    return buf;
}

void unmarshallArgs(char *msg, int *argTypes, void **args) {
    char *msgptr = msg;
    char intbuf[SIZE_INT], shortbuf[SIZE_SHORT], longbuf[SIZE_LONG];
    char floatbuf[SIZE_FLOAT], doublebuf[SIZE_DOUBLE];
    int argType, type, arglen;
    vector<int> types;

    // unmarshall argTypes
    while (1) {
        copy(intbuf, (void *) msgptr, SIZE_INT);
        msgptr += SIZE_INT;
        argType = ctoi(intbuf);
        types.push_back(argType);
        if (argType == ARG_TERMINATOR) break;
    }

    argTypes = new int[types.size()];

    for (unsigned int i = 0; i < types.size(); i++) {
        argTypes[i] = types[i];
    }

    // unmarshall args
    char *carg; short *sarg; int *iarg; long *larg; float *farg; double *darg;
    args = new void*[types.size()-1];

    for (unsigned int i = 0; i < types.size()-1; i++) {
        argType = argTypes[i];
        type = (argType & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        arglen = argType & ARG_LEN_MASK;
        arglen = arglen > 0 ? arglen : 1;

        switch (type) {
            case ARG_CHAR:
                carg = new char[arglen];
                for (int j = 0; j < arglen; j++) {
                    carg[j] = *msgptr;
                    msgptr += SIZE_CHAR;
                }
                args[i] = (void *) carg;
                break;
            case ARG_SHORT:
                sarg = new short[arglen];
                for (int j = 0; j < arglen; j++) {
                    copy(shortbuf, (void *) msgptr, SIZE_SHORT);
                    sarg[j] = ctos(shortbuf);
                    msgptr += SIZE_SHORT;
                }
                args[i] = (void *) sarg;
                break;
            case ARG_INT:
                iarg = new int[arglen];
                for (int j = 0; j < arglen; j++) {
                    copy(intbuf, (void *) msgptr, SIZE_INT);
                    iarg[j] = ctos(intbuf);
                    msgptr += SIZE_INT;
                }
                args[i] = (void *) iarg;
                break;
            case ARG_LONG:
                larg = new long[arglen];
                for (int j = 0; j < arglen; j++) {
                    copy(longbuf, (void *) msgptr, SIZE_LONG);
                    larg[j] = ctos(longbuf);
                    msgptr += SIZE_LONG;
                }
                args[i] = (void *) larg;
                break;
            case ARG_FLOAT:
                farg = new float[arglen];
                for (int j = 0; j < arglen; j++) {
                    copy(floatbuf, (void *) msgptr, SIZE_FLOAT);
                    farg[j] = ctos(floatbuf);
                    msgptr += SIZE_FLOAT;
                }
                args[i] = (void *) farg;
                break;
            case ARG_DOUBLE:
                darg = new double[arglen];
                for (int j = 0; j < arglen; j++) {
                    copy(doublebuf, (void *) msgptr, SIZE_DOUBLE);
                    darg[j] = ctos(doublebuf);
                    msgptr += SIZE_DOUBLE;
                }
                args[i] = (void *) darg;
                break;
        }
    }
}
