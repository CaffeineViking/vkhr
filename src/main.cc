#include <iostream>

#ifndef SHARE_PATH
#define SHARE_PATH "share/"
#endif
#define PATH(X) SHARE_PATH X

int main(int, char**) {
    std::cout << "Hello, world!"
              << std::endl;
    return 0;
}
