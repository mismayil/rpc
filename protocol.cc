#include <iostream>
#include <vector>
#include <cstring>
#include "util.h"
#include "protocol.h"

using namespace std;

SEGMENT::SEGMENT(int type, MESSAGE *message) : len(0), buf(NULL), length(0), type(type), message(message) {}

SEGMENT::~SEGMENT() {
    if (buf) delete [] buf;
    if (message) delete message;
}

int SEGMENT::encapsulate() {
    // INFO("in SEGMENT encapsulate");
    message->marshall();
    char *msgbuf = message->getbuf();
    int msglen = message->getlen();
    length = SIZE_INT + msglen + 1;
    // DEBUG("segment length", length);

    len = length + SIZE_INT;
    buf = new char[len];
    memset(buf, 0, len);
    char *bufptr = buf;

    memcpy(bufptr, &length, SIZE_INT);
    bufptr += SIZE_INT;

    memcpy(bufptr, &type, SIZE_INT);
    bufptr += SIZE_INT;

    memcpy(bufptr, msgbuf, msglen);

    buf[len-1] = NULL_TERMINATOR;

    return RETURN_SUCCESS;
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

int SEGMENT::getlen() { return len; }

char* SEGMENT::getbuf() { return buf; }

MESSAGE::~MESSAGE() {
    if (buf) delete [] buf;
}

MESSAGE::MESSAGE(): len(0), buf(NULL) {}

MESSAGE* MESSAGE::unmarshall(char *msg) { return NULL; }

int MESSAGE::getlen() { return len; }

char* MESSAGE::getbuf() { return buf; }

REQ_REG_MESSAGE::REQ_REG_MESSAGE(char *serverID, int port, char *name, int *argTypes) : serverID(serverID), port(port), name(name), argTypes(argTypes) {}

int REQ_REG_MESSAGE::marshall() {
    // INFO("in REQ_REG_MESSAGE marshall");
    int argc = 1;

    while (argTypes[argc-1] != ARG_TERMINATOR) {
        argc++;
    }

    // DEBUG("argc", argc);

    len = MAX_SERVER_NAME_LEN + SIZE_INT + MAX_FUNC_NAME_LEN + argc * SIZE_INT;
    buf = new char[len];
    memset(buf, 0, len);
    char *bufptr = buf;

    // marshall serverID
    memcpy(bufptr, serverID, strlen(serverID));
    bufptr += MAX_SERVER_NAME_LEN;

    // marshall port
    memcpy(bufptr, &port, SIZE_INT);
    bufptr += SIZE_INT;

    // marshall name
    memcpy(bufptr, name, strlen(name));
    bufptr += MAX_FUNC_NAME_LEN;

    // marshall argTypes
    memcpy(bufptr, argTypes, argc * SIZE_INT);

    return RETURN_SUCCESS;
}

MESSAGE* REQ_REG_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    char intbuf[SIZE_INT];
    char *serverID;
    int port, argType;
    char *name;
    int *argTypes;
    vector<int> types;

    // unmarshall server identifier
    serverID = new char[MAX_SERVER_NAME_LEN];
    memcpy(serverID, msgptr, MAX_SERVER_NAME_LEN);
    msgptr += MAX_SERVER_NAME_LEN;

    // unmarshall port
    memcpy(intbuf, msgptr, SIZE_INT);
    port = ctoi(intbuf);
    msgptr += SIZE_INT;

    // unmarshall name
    name = new char[MAX_FUNC_NAME_LEN];
    memcpy(name, msgptr, MAX_FUNC_NAME_LEN);
    msgptr += MAX_FUNC_NAME_LEN;

    // unmarshall arg types
    while (1) {
        memcpy(intbuf, msgptr, SIZE_INT);
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

int RES_REG_SUCCESS_MESSAGE::marshall() {
    // marshall reasonCode
    len = SIZE_INT;
    buf = new char[len];
    memset(buf, 0, len);
    memcpy(buf, &reasonCode, SIZE_INT);
    return RETURN_SUCCESS;
}

MESSAGE* RES_REG_SUCCESS_MESSAGE::unmarshall(char *msg) {
    // unmarshall reasonCode
    int reasonCode;
    reasonCode = ctoi(msg);
    MESSAGE *message = new RES_REG_SUCCESS_MESSAGE(reasonCode);
    return message;
}

REQ_LOC_MESSAGE::REQ_LOC_MESSAGE(char *name, int *argTypes) : name(name), argTypes(argTypes) {}

int REQ_LOC_MESSAGE::marshall() {
    int argc = 1;

    while (argTypes[argc-1] != ARG_TERMINATOR) {
        argc++;
    }

    len = MAX_FUNC_NAME_LEN + argc * SIZE_INT;
    buf = new char[len];
    memset(buf, 0, len);
    char *bufptr = buf;

    // marshall name
    memcpy(bufptr, name, strlen(name));
    bufptr += MAX_FUNC_NAME_LEN;

    // marshall argTypes
    memcpy(bufptr, argTypes, argc * SIZE_INT);

    return RETURN_SUCCESS;
}

MESSAGE* REQ_LOC_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    char intbuf[SIZE_INT];
    int argType;
    char *name;
    int *argTypes;
    vector<int> types;

    // unmarshall name
    name = new char[MAX_FUNC_NAME_LEN];
    memcpy(name, msgptr, MAX_FUNC_NAME_LEN);
    msgptr += MAX_FUNC_NAME_LEN;

    // unmarshall arg types
    while (1) {
        memcpy(intbuf, msgptr, SIZE_INT);
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

int RES_LOC_SUCCESS_MESSAGE::marshall() {
    len = MAX_SERVER_NAME_LEN + SIZE_INT;
    buf = new char[len];
    memset(buf, 0, len);
    char *bufptr = buf;

    // marshall serverID
    memcpy(bufptr, serverID, strlen(serverID));
    bufptr += MAX_SERVER_NAME_LEN;

    // marshall port
    memcpy(bufptr, &port, SIZE_INT);

    return RETURN_SUCCESS;
}

MESSAGE* RES_LOC_SUCCESS_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    char intbuf[SIZE_INT];
    char *serverID;
    int port;

    // unmarshall server identifier
    serverID = new char[MAX_SERVER_NAME_LEN];
    memcpy(serverID, msgptr, MAX_SERVER_NAME_LEN);
    msgptr += MAX_SERVER_NAME_LEN;

    // unmarshall port
    memcpy(intbuf, msgptr, SIZE_INT);
    port = ctoi(intbuf);

    MESSAGE* message = new RES_LOC_SUCCESS_MESSAGE(serverID, port);

    return message;
}

REQ_EXEC_MESSAGE::REQ_EXEC_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

int REQ_EXEC_MESSAGE::marshall() {
    // marshall argTypes and args
    int argbuflen = 0;
    char *argbuf = NULL;
    marshallArgs(argTypes, args, &argbuf, &argbuflen);

    len = MAX_FUNC_NAME_LEN + argbuflen;
    buf = new char[len];
    memset(buf, 0, len);
    char *bufptr = buf;

    // marshall name
    memcpy(bufptr, name, strlen(name));
    bufptr += MAX_FUNC_NAME_LEN;

    memcpy(bufptr, argbuf, argbuflen);

    return RETURN_SUCCESS;
}

MESSAGE* REQ_EXEC_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    char *name;
    int *argTypes = NULL;
    void **args = NULL;

    // unmarshall name
    name = new char[MAX_FUNC_NAME_LEN];
    memcpy(name, msgptr, MAX_FUNC_NAME_LEN);
    msgptr += MAX_FUNC_NAME_LEN;

    // unmarshall arg types and args
    unmarshallArgs(msgptr, &argTypes, &args);

    MESSAGE* message = new REQ_EXEC_MESSAGE(name, argTypes, args);

    return message;
}

int REQ_TERM_MESSAGE::marshall() {
    len = 0;
    buf = new char[len];
    memset(buf, 0, len);
    return RETURN_SUCCESS;
}

MESSAGE* REQ_TERM_MESSAGE::unmarshall(char *msg) {
    return new REQ_TERM_MESSAGE();
}

RES_EXEC_SUCCESS_MESSAGE::RES_EXEC_SUCCESS_MESSAGE(char *name, int *argTypes, void **args) : name(name), argTypes(argTypes), args(args) {}

int RES_EXEC_SUCCESS_MESSAGE::marshall() {
    // marshall argTypes and args
    int argbuflen = 0;
    char *argbuf = NULL;
    marshallArgs(argTypes, args, &argbuf, &argbuflen);

    len = MAX_FUNC_NAME_LEN + argbuflen;
    buf = new char[len];
    memset(buf, 0, len);
    char *bufptr = buf;

    // marshall name
    memcpy(bufptr, name, strlen(name));
    bufptr += MAX_FUNC_NAME_LEN;

    memcpy(bufptr, argbuf, argbuflen);

    return RETURN_SUCCESS;
}

MESSAGE* RES_EXEC_SUCCESS_MESSAGE::unmarshall(char *msg) {
    char *msgptr = msg;
    char *name;
    int *argTypes = NULL;
    void **args = NULL;

    // unmarshall name
    name = new char[MAX_FUNC_NAME_LEN];
    memcpy(name, msgptr, MAX_FUNC_NAME_LEN);
    msgptr += MAX_FUNC_NAME_LEN;

    // unmarshall arg types and args
    unmarshallArgs(msgptr, &argTypes, &args);

    MESSAGE* message = new RES_EXEC_SUCCESS_MESSAGE(name, argTypes, args);

    return message;
}

RES_FAILURE_MESSAGE::RES_FAILURE_MESSAGE(int reasonCode) : reasonCode(reasonCode) {}

int RES_FAILURE_MESSAGE::marshall() {
    // marshall reasonCode
    len = SIZE_INT;
    buf = new char[len];
    memset(buf, 0, len);

    memcpy(buf, &reasonCode, SIZE_INT);
    return RETURN_SUCCESS;
}

MESSAGE* RES_FAILURE_MESSAGE::unmarshall(char *msg) {
    // unmarshall reasonCode
    int reasonCode;
    reasonCode = ctoi(msg);
    MESSAGE *message = new RES_FAILURE_MESSAGE(reasonCode);
    return message;
}

int marshallArgs(int *argTypes, void **args, char **buf, int *buflen) {
    int arglen, type, argType, argc = 1;

    while (argTypes[argc-1] != ARG_TERMINATOR) {
        argc++;
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

    *buf = new char[len];
    *buflen = len;
    memset(*buf, 0, len);
    char *bufptr = *buf;

    memcpy(bufptr, argTypes, argc * SIZE_INT);
    bufptr += argc * SIZE_INT;

    for (int i = 0; i < argc - 1; i++) {
        argType = argTypes[i];
        type = (argType & ARG_TYPE_MASK) >> ARG_TYPE_SHIFT;
        arglen = argType & ARG_LEN_MASK;
        arglen = arglen > 0 ? arglen : 1;

        switch (type) {
            case ARG_CHAR:
                memcpy(bufptr, args[i], arglen * SIZE_CHAR);
                bufptr += arglen * SIZE_CHAR;
                break;
            case ARG_SHORT:
                memcpy(bufptr, args[i], arglen * SIZE_SHORT);
                bufptr += arglen * SIZE_SHORT;
                break;
            case ARG_INT:
                memcpy(bufptr, args[i], arglen * SIZE_INT);
                bufptr += arglen * SIZE_INT;
                break;
            case ARG_LONG:
                memcpy(bufptr, args[i], arglen * SIZE_LONG);
                bufptr += arglen * SIZE_LONG;
                break;
            case ARG_FLOAT:
                memcpy(bufptr, args[i], arglen * SIZE_FLOAT);
                bufptr += arglen * SIZE_FLOAT;
                break;
            case ARG_DOUBLE:
                memcpy(bufptr, args[i], arglen * SIZE_DOUBLE);
                bufptr += arglen * SIZE_DOUBLE;
                break;
        }
    }

    return RETURN_SUCCESS;
}

int unmarshallArgs(char *msg, int **argTypes, void ***args) {
    char *msgptr = msg;
    char intbuf[SIZE_INT], shortbuf[SIZE_SHORT], longbuf[SIZE_LONG];
    char floatbuf[SIZE_FLOAT], doublebuf[SIZE_DOUBLE];
    int argType, type, arglen;
    vector<int> types;

    // unmarshall argTypes
    while (1) {
        memcpy(intbuf, msgptr, SIZE_INT);
        msgptr += SIZE_INT;
        argType = ctoi(intbuf);
        types.push_back(argType);
        if (argType == ARG_TERMINATOR) break;
    }

    *argTypes = new int[types.size()];

    for (unsigned int i = 0; i < types.size(); i++) {
        (*argTypes)[i] = types[i];
    }

    // unmarshall args
    char *carg; short *sarg; int *iarg; long *larg; float *farg; double *darg;
    *args = new void*[types.size()-1];

    for (unsigned int i = 0; i < types.size()-1; i++) {
        argType = (*argTypes)[i];
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
                (*args)[i] = carg;
                break;
            case ARG_SHORT:
                sarg = new short[arglen];
                for (int j = 0; j < arglen; j++) {
                    memcpy(shortbuf, msgptr, SIZE_SHORT);
                    sarg[j] = ctos(shortbuf);
                    msgptr += SIZE_SHORT;
                }
                (*args)[i] = sarg;
                break;
            case ARG_INT:
                iarg = new int[arglen];
                for (int j = 0; j < arglen; j++) {
                    memcpy(intbuf, msgptr, SIZE_INT);
                    iarg[j] = ctoi(intbuf);
                    msgptr += SIZE_INT;
                }
                (*args)[i] = iarg;
                break;
            case ARG_LONG:
                larg = new long[arglen];
                for (int j = 0; j < arglen; j++) {
                    memcpy(longbuf, msgptr, SIZE_LONG);
                    larg[j] = ctol(longbuf);
                    msgptr += SIZE_LONG;
                }
                (*args)[i] = larg;
                break;
            case ARG_FLOAT:
                farg = new float[arglen];
                for (int j = 0; j < arglen; j++) {
                    memcpy(floatbuf, msgptr, SIZE_FLOAT);
                    farg[j] = ctof(floatbuf);
                    msgptr += SIZE_FLOAT;
                }
                (*args)[i] = farg;
                break;
            case ARG_DOUBLE:
                darg = new double[arglen];
                for (int j = 0; j < arglen; j++) {
                    memcpy(doublebuf, msgptr, SIZE_DOUBLE);
                    darg[j] = ctod(doublebuf);
                    msgptr += SIZE_DOUBLE;
                }
                (*args)[i] = darg;
                break;
        }
    }

    return RETURN_SUCCESS;
}
