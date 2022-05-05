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

#include "Buffer.h"

#include <sys/uio.h>

namespace wind {
namespace conn {
Buffer::Buffer(std::size_t initialSize, std::size_t prependSize)
    : data_(prependSize + initialSize), readIdx_(prependSize), writeIdx_(prependSize)
{}

void Buffer::swap(Buffer &other)
{
    data_.swap(other.data_);
    std::swap(readIdx_, other.readIdx_);
    std::swap(writeIdx_, other.writeIdx_);
}

Buffer::Buffer(Buffer &&other) noexcept
    : data_(std::move(other.data_)), readIdx_(other.readIdx_), writeIdx_(other.writeIdx_)
{}

Buffer &Buffer::operator=(Buffer &&other) noexcept
{
    swap(other);
    return *this;
}

void Buffer::resume(std::size_t len)
{
    ASSERT(len <= bytesReadable());
    if (WIND_LIKELY(len < bytesReadable())) {
        readIdx_ += len;
    } else {
        resumeAll();
    }
}

void Buffer::makeMoreSpace(std::size_t len)
{
    // make more space or move the data to the front inside this buffer.
    if ((len + PREPEND_SIZE) > (bytesWritable() + prependBytes())) {
        // make more space.
        std::vector<char> newBuf(writeIdx_ + len);
        std::move(data_.begin(), data_.end(), newBuf.begin());
        data_ = std::move(newBuf);
    } else {
        // move the data to the front inside this buffer.
        ASSERT(readIdx_ <= writeIdx_);
        ASSERT(readIdx_ > PREPEND_SIZE);
        std::move(begin() + readIdx_, begin() + writeIdx_, begin() + PREPEND_SIZE);

        std::size_t readable = bytesReadable();
        readIdx_ = PREPEND_SIZE;
        writeIdx_ = readable + readIdx_;
        ASSERT(readable == bytesReadable());
    }
}

void Buffer::append(const char *data, std::size_t len)
{
    if (bytesWritable() < len) {
        makeMoreSpace(len);
    }

    ASSERT(bytesWritable() >= len);
    std::copy(data, data + len, writeBegin());
    writeIdx_ += len;
}

// See muduo's Buffer::readFd() in https://github.com/chenshuo/muduo/blob/master/muduo/net/Buffer.cc
ssize_t Buffer::handleSocketRead(int sockFd, int &savedErrno)
{
    char extraBuf[65536];
    iovec vec[2];
    const std::size_t writable = bytesWritable();
    vec[0].iov_base = begin() + writeIdx_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extraBuf;
    vec[1].iov_len = sizeof(extraBuf);
    // when there is enough space in this buffer, don't read into extraBuf.
    const int iovCnt = (writable < sizeof(extraBuf)) ? 2 : 1;
    const auto len = ::readv(sockFd, vec, iovCnt);
    if (len < 0) {
        savedErrno = errno;
    } else if (static_cast<std::size_t>(len) <= writable) {
        writeIdx_ += len;
    } else {
        writeIdx_ = data_.size();
        append(extraBuf, len - writable);
    }

    return len;
}
} // namespace conn
} // namespace wind
