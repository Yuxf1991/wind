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

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <sys/epoll.h>

#include "NonCopyable.h"
#include "TimeStamp.h"

namespace wind {
namespace base {
using EventCallback = std::function<void()>;
using ReadCallback = std::function<void(TimeStamp)>;

class EventPoller;
class EventLoop;

// use EPOLL_EVENTS to define channel event types.
enum class EventType : uint32_t {
    NONE = 0x0,
    READ_EVENT = EPOLLIN | EPOLLPRI,
    WRITE_EVENT = EPOLLOUT,
    EDGE_EVENT = EPOLLET,
};

// The EventChannel class not own the fd, so we must make sure that
// the fd is valid when an EventChannel object holds it.
class EventChannel : public std::enable_shared_from_this<EventChannel>, NonCopyable {
public:
    EventChannel(int fd, EventLoop *eventLoop);
    ~EventChannel() noexcept;

    void handleEvent(TimeStamp receivedTime);

    int fd() const
    {
        return fd_;
    }
    EventLoop *getOwnerLoop() const
    {
        return eventLoop_;
    }

    // not thread safe.
    void setReadCallback(ReadCallback cb)
    {
        readCallback_ = std::move(cb);
    }
    // not thread safe.
    void setWriteCallback(EventCallback cb)
    {
        writeCallback_ = std::move(cb);
    }
    // not thread safe.
    void setErrorCallback(EventCallback cb)
    {
        errorCallback_ = std::move(cb);
    }
    // not thread safe.
    void setCloseCallback(EventCallback cb)
    {
        closeCallback_ = std::move(cb);
    }

    bool hasNoEvent() const
    {
        return listeningEvents() == enum_cast(EventType::NONE);
    }
    bool isWriting() const
    {
        return listeningEvents() & enum_cast(EventType::WRITE_EVENT);
    }
    bool isReading() const
    {
        return listeningEvents() & enum_cast(EventType::READ_EVENT);
    }

    // @toUpdate: whether to update the channel in poller or not, true by default.
    void enableReading(bool toUpdate = true);
    // @toUpdate: whether to update the channel in poller or not, true by default.
    void disableReading(bool toUpdate = true);
    // @toUpdate: whether to update the channel in poller or not, true by default.
    void enableWriting(bool toUpdate = true);
    // @toUpdate: whether to update the channel in poller or not, true by default.
    void disableWriting(bool toUpdate = true);
    // @toRemove: whether to remove the channel in poller or not, true by default.
    void disableAll(bool toRemove = true);

    void update();
    void remove();

    // the owner object should call this function
    // to prevent itself from being destroyed in handleEvent.
    void tie(std::weak_ptr<void> ownerObj)
    {
        ownerObj_ = ownerObj;
        tied_ = true;
    }

protected:
    // will abort if not in loop thread.
    void assertInLoopThread() const;

    void handleEventInner(TimeStamp receivedTime);

    friend class EventPoller;
    uint32_t listeningEvents() const
    {
        return listeningEvents_;
    }
    void setReceivedEvents(uint32_t events)
    {
        receivedEvents_ = events;
    }

    int fd_ = -1;
    EventLoop *eventLoop_ = nullptr;
    std::atomic<bool> addedToLoop_ = false;

    uint32_t listeningEvents_ = enum_cast(EventType::NONE);
    uint32_t receivedEvents_ = enum_cast(EventType::NONE);

    ReadCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
    EventCallback closeCallback_;

    std::weak_ptr<void> ownerObj_;
    std::atomic<bool> tied_ = false;
};
} // namespace base
} // namespace wind
