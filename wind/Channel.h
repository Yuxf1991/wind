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

#ifndef WIND_CHANNEL_H
#define WIND_CHANNEL_H

#include <functional>

#include "NonCopyable.h"
#include "UniqueFd.h"

namespace wind {
using ChannelCallback = std::function<void()>;

enum class ChannelEvent : uint32_t {
    NONE = 0x0,
    READ_EVNET = 0x1,
    EMERGENCY_EVENT = 0x2,
    WRITE_EVENT = 0x4,
    READ_AND_WRITE_EVENT = READ_EVNET | WRITE_EVENT,
    EMERGENCY_READ_EVENT = READ_EVNET | EMERGENCY_EVENT,
    EMERGENCY_WRITE_EVENT = WRITE_EVENT | EMERGENCY_EVENT,
    EMERGENCY_READ_AND_WRITE_EVENT = READ_EVNET | WRITE_EVENT | EMERGENCY_EVENT,
};

class Channel : NonCopyable {
public:
    Channel(int fd, ChannelCallback callback);
    virtual ~Channel() noexcept;

    int fd() const
    {
        return fd_.get();
    }

    ChannelEvent events() const
    {
        return events_;
    }
    virtual void dispatch() const;

protected:
    UniqueFd fd_;
    ChannelEvent events_ = ChannelEvent::EMERGENCY_READ_AND_WRITE_EVENT;
    ChannelCallback callback_;
};
} // namespace wind
#endif // WIND_CHANNEL_H
