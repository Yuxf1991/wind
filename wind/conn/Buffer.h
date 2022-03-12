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

#include <vector>

#include "base/Types.h"
#include "base/Utils.h"

namespace wind {
namespace conn {
// copyable and movable
class Buffer {
public:
    static constexpr size_t PREPEND_SIZE = 8;
    static constexpr size_t INITIAL_SIZE = 1016;

    explicit Buffer(size_t initialSize = INITIAL_SIZE, size_t prependSize = PREPEND_SIZE);
    ~Buffer() noexcept = default;

    Buffer(const Buffer &other) = default;
    Buffer &operator=(const Buffer &other) = default;

    void swap(Buffer &other);

    Buffer(Buffer &&other) noexcept;
    Buffer &operator=(Buffer &&other) noexcept;

    void retrieveAll()
    {
        readIdx_ = PREPEND_SIZE;
        writeIdx_ = readIdx_;
    }
    string readAll();
    void append(const char *data, size_t len);
    template <typename T>
    void append(const T *data, size_t len)
    {
        append(static_cast<const char *>(data), len);
    }

    const char *begin() const
    {
        return data_.data();
    }
    char *begin()
    {
        return const_cast<char *>(static_cast<const Buffer &>(*this).begin());
    }
    const char *peek() const
    {
        return begin() + readIdx_;
    }
    const char *writeBegin() const
    {
        return begin() + writeIdx_;
    }
    char *writeBegin()
    {
        return const_cast<char *>(static_cast<const Buffer &>(*this).writeBegin());
    }
    size_t bytesPrepend() const
    {
        return readIdx_;
    }
    size_t bytesReadable() const
    {
        ASSERT(readIdx_ <= writeIdx_);
        return writeIdx_ - readIdx_;
    }
    size_t bytesWritable() const
    {
        ASSERT(writeIdx_ <= data_.size());
        return data_.size() - writeIdx_;
    }

private:
    // make more space if necessary
    void makeMoreSpace(size_t len);
    std::vector<char> data_;
    size_t readIdx_ = PREPEND_SIZE;
    size_t writeIdx_ = PREPEND_SIZE;
};
} // namespace conn
} // namespace wind
