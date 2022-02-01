#include <iostream>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include "TimeStamp.h"

using namespace std;

void PrintTime()
{
    cout << wind::TimeStamp::now().toFormattedString() << endl;
    cout << wind::TimeStamp::now().get() << endl;
}

int main()
{
    int epollFd = epoll_create1(EPOLL_CLOEXEC);
    int timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    itimerspec newValue;
    newValue.it_value.tv_sec = 1;
    newValue.it_value.tv_nsec = 0;
    newValue.it_interval.tv_sec = 5;
    newValue.it_interval.tv_nsec = 0;
    timerfd_settime(timerFd, 0, &newValue, NULL);
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.ptr = (void *)(&PrintTime);
    (void)epoll_ctl(epollFd, EPOLL_CTL_ADD, timerFd, &ev);

    while (true) {
        epoll_event evs[10];
        int ret = TEMP_FAILURE_RETRY(epoll_wait(epollFd, evs, 10, -1));
        if (ret > 0) {
            for (int i = 0; i < ret; ++i) {
                uint64_t tmp;
                (void)TEMP_FAILURE_RETRY(read(timerFd, &tmp, sizeof(tmp)));
                cout << tmp << endl;
                auto func = (void (*)())(evs[i].data.ptr);
                func();
            }
        } else if (ret < 0) {
            break;
        }
    }

    return 0;
}
