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

#ifndef WIND_EVENT_CHANNEL_H
#define WIND_EVENT_CHANNEL_H

#include <functional>
#include <memory>
#include <sys/epoll.h>

#include "TimeStamp.h"
#include "UniqueFd.h"

namespace wind {
using EventCallback = std::function<void()>;
using EventCallbackWithStamp = std::function<void(TimeStamp)>;

class EventPoller;
class EventLoop;

// use EPOLL_EVENTS to define channel event types.
enum class EventType : uint32_t {
    NONE = 0x0,
    READ_EVNET = EPOLLIN | EPOLLPRI,
    WRITE_EVENT = EPOLLOUT,
    EDGE_EVENT = EPOLLET,
};

class EventChannel : NonCopyable {
public:
    EventChannel(int fd, EventLoop *eventLoop);
    virtual ~EventChannel() noexcept;

    int fd() const { return fd_.get(); }

    void setReadCallback(EventCallbackWithStamp cb)
    {
        readCallback_ = std::move(cb);
        update();
    }

    void setWriteCallback(EventCallback cb)
    {
        writeCallback_ = std::move(cb);
        update();
    }

    void setErrorCallback(EventCallback cb)
    {
        errorCallback_ = std::move(cb);
        update();
    }

    void setCloseCallback(EventCallback cb)
    {
        closeCallback_ = std::move(cb);
        update();
    }

    virtual void handleEvent(TimeStamp receivedTime);

protected:
    friend class EventPoller;
    uint32_t eventsToHandle() const { return eventsToHandle_; }

    void setRecevicedEvents(uint32_t events) { receivedEvents_ = events; }

    void update();
    void enableReading();
    void disableReading();
    void enableWriting();
    void disableWriting();

    UniqueFd fd_;
    EventLoop *eventLoop_ = nullptr;
    uint32_t eventsToHandle_ = enum_cast(EventType::NONE);
    uint32_t receivedEvents_ = enum_cast(EventType::NONE);

    EventCallbackWithStamp readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;
};
} // namespace wind
#endif // WIND_EVENT_CHANNEL_H
