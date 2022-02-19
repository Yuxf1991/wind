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

int main()
{
    SockAddrInet addr0;
    cout << addr0.toString() << endl;

    SockAddrInet addr1("0.0.0.0", 1645);
    cout << addr1.toString() << endl;

    SockAddrInet addr2(INADDR_ANY, 8888);
    cout << addr2.toString() << endl;

    SockAddrInet addr3("255.255.255.255", 1645);
    cout << addr3.toString() << endl;

    SockAddrInet addr4("127.0.0.1", 12459);
    cout << addr4.toString() << endl;
    cout << addr4.len() << endl;

    // SockAddrInet addr5("127.0.0.1457", 12459); // not valid addr
    // SockAddrInet addr6("256.0.0.1", 12459); // not valid addr
    // SockAddrInet addr7("1.-41.0.1", 154); // not valid addr
    // SockAddrInet addr8("10.41.652.1", 154); // not valid addr
    return 0;
}
