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

#include "EventChannel.h"

#include "EventLoop.h"
#include "Log.h"
#include "Utils.h"

namespace wind {
namespace base {
EventChannel::EventChannel(int fd, EventLoop *eventLoop)
    : std::enable_shared_from_this<EventChannel>(), fd_(fd), eventLoop_(eventLoop)
{
    LOG_FATAL_IF(eventLoop_ == nullptr) << "EventLoop is null!";
}

EventChannel::~EventChannel() noexcept {}

void EventChannel::assertInLoopThread() const
{
    ASSERT(eventLoop_ != nullptr);
    eventLoop_->assertInLoopThread();
}

void EventChannel::remove()
{
    ASSERT(hasNoEvent());
    addedToLoop_ = false;
    eventLoop_->removeChannel(fd_);
}

void EventChannel::enableReading(bool toUpdate)
{
    listeningEvents_ |= enum_cast(EventType::READ_EVNET);

    if (toUpdate) {
        update();
    }
}

void EventChannel::disableReading(bool toUpdate)
{
    listeningEvents_ &= (~enum_cast(EventType::READ_EVNET));

    if (toUpdate) {
        update();
    }
}

void EventChannel::enableWriting(bool toUpdate)
{
    listeningEvents_ |= enum_cast(EventType::WRITE_EVENT);

    if (toUpdate) {
        update();
    }
}

void EventChannel::disableWriting(bool toUpdate)
{
    listeningEvents_ &= (~enum_cast(EventType::WRITE_EVENT));

    if (toUpdate) {
        update();
    }
}

void EventChannel::disableAll(bool toRemove)
{
    listeningEvents_ = enum_cast(EventType::NONE);

    if (toRemove) {
        remove();
    }
}

void EventChannel::update()
{
    if (hasNoEvent()) {
        remove();
    } else {
        addedToLoop_ = true;
        eventLoop_->updateChannel(shared_from_this());
    }
}

void EventChannel::handleEvent(TimeStamp receivedTime)
{
    assertInLoopThread();

    if ((receivedEvents_ & EPOLLHUP) && !(receivedEvents_ & EPOLLIN)) {
        LOG_TRACE << "close event in channel " << fd_ << ".";
        auto closeCallback = getCloseCallback();
        if (closeCallback != nullptr) {
            closeCallback();
            LOG_DEBUG << "Remove channel fd " << fd_ << " from poller.";
            disableAll();
            return;
        }
    }

    if (receivedEvents_ & EPOLLERR) {
        LOG_TRACE << "error event in channel " << fd_ << ".";
        auto errorCallback = getErrorCallback();
        if (errorCallback != nullptr) {
            errorCallback();
            LOG_DEBUG << "Remove channel fd " << fd_ << " from poller.";
            disableAll();
            return;
        }
    }

    if (receivedEvents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        LOG_TRACE << "read event in channel " << fd_ << ".";
        auto readCallback = getReadCallback();
        if (readCallback != nullptr) {
            readCallback(receivedTime);
        }
    }

    if (receivedEvents_ & EPOLLOUT) {
        LOG_TRACE << "write event in channel " << fd_ << ".";
        auto writeCallback = getWriteCallback();
        if (writeCallback != nullptr) {
            writeCallback();
        }
    }
}
} // namespace base
} // namespace wind
