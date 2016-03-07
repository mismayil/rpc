#include <iostream>
#include <cstring>

using namespace std;

void foo(char **buf) {
    *buf = new char[5];
    char str[6] = "hello";
    memcpy(*buf, str, 6);
    //cout << buf << endl;
}

int main() {
    char *buf = new char[0];
    cout << buf << endl;
}
