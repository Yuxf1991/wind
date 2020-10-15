//
// Created by tracy on 2020/10/15.
//

#ifndef WIND_SOCKET_H
#define WIND_SOCKET_H

#include "NonCopyable.h"

namespace wind {
    namespace sockets {
        int createCloExec(int domain);
    } // namespace sockets

    class Socket {
    public:
        explicit Socket(int socket);
        ~Socket();

    private:
        int m_socket;
    };
} // namespace wind

#endif //WIND_SOCKET_H
