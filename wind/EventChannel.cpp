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
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "EventChannel"
#endif // LOG_TAG
#include "Log.h"
#include "Utils.h"

namespace wind {
EventChannel::EventChannel(int fd, EventLoop *eventLoop) : fd_(fd), eventLoop_(eventLoop)
{
    LOG_FATAL_IF(eventLoop_ == nullptr) << "EventLoop is null!";
}

EventChannel::~EventChannel() noexcept {}

void EventChannel::enableReading()
{
    eventsToHandle_ |= enum_cast(EventType::READ_EVNET);
}

void EventChannel::disableReading()
{
    eventsToHandle_ &= (~enum_cast(EventType::READ_EVNET));
}

void EventChannel::enableWriting()
{
    eventsToHandle_ |= enum_cast(EventType::WRITE_EVENT);
}

void EventChannel::disableWriting()
{
    eventsToHandle_ &= (~enum_cast(EventType::WRITE_EVENT));
}

void EventChannel::update()
{
    uint32_t oldEventsToHandle = eventsToHandle_;
    if (readCallback_ == nullptr) {
        disableReading();
    } else {
        enableReading();
    }

    if (writeCallback_ == nullptr) {
        disableWriting();
    } else {
        enableWriting();
    }

    if (oldEventsToHandle != eventsToHandle_) {
        eventLoop_->updateChannel(shared_from_this());
    }
}

void EventChannel::handleEvent(TimeStamp receivedTime) const
{
    eventLoop_->assertInLoopThread();

    if ((receivedEvents_ & EPOLLHUP) && !(receivedEvents_ & EPOLLIN)) {
        if (closeCallback_ != nullptr) {
            closeCallback_();
        }
    }

    if (receivedEvents_ & EPOLLERR) {
        if (errorCallback_ != nullptr) {
            errorCallback_();
        }
    }

    if (receivedEvents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readCallback_ != nullptr) {
            readCallback_(receivedTime);
        }
    }

    if (receivedEvents_ & EPOLLOUT) {
        if (writeCallback_ != nullptr) {
            writeCallback_();
        }
    }
}
} // namespace wind
