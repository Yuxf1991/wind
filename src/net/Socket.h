//
// Created by tracy on 2020/10/15.
//

#ifndef WIND_SOCKET_H
#define WIND_SOCKET_H

#include "Utils.h"

namespace wind {
    namespace sockets {
        int createCloExec(int domain);
    } // namespace sockets

    class Socket : public utils::ScopedFd {
    public:
        explicit Socket(int fd);
        ~Socket() override;
    };
} // namespace wind

#endif //WIND_SOCKET_H
