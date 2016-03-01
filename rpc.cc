#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread>
#include "rpc.h"

using namespace std;

int rpcInit();
int rpcCall(char* name, int* argTypes, void** args);
int rpcCacheCall(char* name, int* argTypes, void** args);
int rpcRegister(char* name, int* argTypes, skeleton f);
int rpcExecute();
int rpcTerminate();
