#include <iostream>
#include <cstring>

using namespace std;

int main() {
    int a = 56;
    char intbuf[4];
    memcpy(intbuf, &a, 4);
    int *b = (int *) intbuf;
    cout << *b << endl;
}
