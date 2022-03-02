// MIT License

// Copyright (c) 2020 Tracy

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "SockAddrInet.h"
#include <iostream>

using namespace std;
using namespace wind;
using namespace wind::network;

int main()
{
    // TODO: Use google test.
    SockAddrInet addr0;
    cout << addr0.ipPortString() << endl;

    SockAddrInet addr1("0.0.0.0", 1645);
    cout << addr1.ipPortString() << endl;

    SockAddrInet addr2(8888, false, true);
    cout << addr2.ipPortString() << endl;

    SockAddrInet addr3("255.255.255.255", 1645);
    cout << addr3.ipPortString() << endl;

    SockAddrInet addr4("127.0.0.1", 12459);
    cout << addr4.ipPortString() << endl;
    cout << addr4.len() << endl;

    SockAddrInet addr5(8888, true, true);
    cout << addr5.ipPortString() << endl;

    SockAddrInet addr6(8888, true);
    cout << addr6.ipPortString() << endl;

    SockAddrInet addr7(8888);
    cout << addr7.ipPortString() << endl;

    // SockAddrInet addr5("127.0.0.1457", 12459); // not valid addr
    // SockAddrInet addr6("256.0.0.1", 12459); // not valid addr
    // SockAddrInet addr7("1.-41.0.1", 154); // not valid addr
    // SockAddrInet addr8("10.41.652.1", 154); // not valid addr
    return 0;
}
