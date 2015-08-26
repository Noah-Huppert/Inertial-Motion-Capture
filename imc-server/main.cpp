#include <iostream>
#include <unistd.h>

using namespace std;

int main() {
    for(int i = 0; i < 10; i++){
        cout << "Hello, World!" << endl;
        sleep(1);
    }

    return 0;
}
