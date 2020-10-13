//
// Created by tracy on 2020/10/13.
//

#include <iostream>

#include "base/Thread.h"

using wind::Thread;
using std::cout;
using std::endl;
using std::cin;

constexpr int MAX_NUM = 1000;

void threadFunc() {
    for (int i = 0; i < MAX_NUM; ++i) {
        cout << i << " ";
    }
    cout << endl;
}

int main() {
    {
        Thread t{[](){ threadFunc(); }};
        t.start();
        if (t.joinable()) {
            t.join();
        }
    }

    cout << "Press any key to quit main thread." << endl;
    std::string tmp;
    getline(cin, tmp);
    return 0;
}
