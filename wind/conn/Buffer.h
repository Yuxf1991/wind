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

#include <type_traits>
#include <vector>

#include "base/Types.h"
#include "base/Utils.h"
#include "Endian.h"

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
    size_t prependBytes() const
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

    void resume(size_t len);
    void resumeAll()
    {
        readIdx_ = PREPEND_SIZE;
        writeIdx_ = readIdx_;
    }

    int8_t peekInt8() const
    {
        ASSERT(bytesReadable() >= sizeof(int8_t));
        int8_t val = 0;
        ::memcpy(&val, peek(), sizeof(int8_t));
        return val;
    }
    uint8_t peekUInt8() const
    {
        ASSERT(bytesReadable() >= sizeof(uint8_t));
        uint8_t val = 0;
        ::memcpy(&val, peek(), sizeof(uint8_t));
        return val;
    }
    int16_t peekInt16() const
    {
        ASSERT(bytesReadable() >= sizeof(int16_t));
        int16_t val = 0;
        ::memcpy(&val, peek(), sizeof(int16_t));
        return endian::netToHost(val);
    }
    uint16_t peekUInt16() const
    {
        ASSERT(bytesReadable() >= sizeof(uint16_t));
        uint16_t val = 0;
        ::memcpy(&val, peek(), sizeof(uint16_t));
        return endian::netToHost(val);
    }
    int32_t peekInt32() const
    {
        ASSERT(bytesReadable() >= sizeof(int32_t));
        int32_t val = 0;
        ::memcpy(&val, peek(), sizeof(int32_t));
        return endian::netToHost(val);
    }
    uint32_t peekUInt32() const
    {
        ASSERT(bytesReadable() >= sizeof(uint32_t));
        uint32_t val = 0;
        ::memcpy(&val, peek(), sizeof(uint32_t));
        return endian::netToHost(val);
    }
    int64_t peekInt64() const
    {
        ASSERT(bytesReadable() >= sizeof(int64_t));
        int64_t val = 0;
        ::memcpy(&val, peek(), sizeof(int64_t));
        return endian::netToHost(val);
    }
    uint64_t peekUInt64() const
    {
        ASSERT(bytesReadable() >= sizeof(uint64_t));
        uint64_t val = 0;
        ::memcpy(&val, peek(), sizeof(uint64_t));
        return endian::netToHost(val);
    }

    int8_t readInt8()
    {
        int8_t val = peekInt8();
        resume(sizeof(int8_t));
        return val;
    }
    uint8_t readUInt8()
    {
        uint8_t val = peekUInt8();
        resume(sizeof(uint8_t));
        return val;
    }
    int16_t readInt16()
    {
        int16_t val = peekInt16();
        resume(sizeof(int16_t));
        return val;
    }
    uint16_t readUInt16()
    {
        uint16_t val = peekUInt16();
        resume(sizeof(uint16_t));
        return val;
    }
    int32_t readInt32()
    {
        int32_t val = peekInt32();
        resume(sizeof(int32_t));
        return val;
    }
    uint32_t readUInt32()
    {
        uint32_t val = peekUInt32();
        resume(sizeof(uint32_t));
        return val;
    }
    int64_t readInt64()
    {
        int64_t val = peekInt64();
        resume(sizeof(int64_t));
        return val;
    }
    uint64_t readUInt64()
    {
        uint64_t val = peekUInt64();
        resume(sizeof(uint64_t));
        return val;
    }
    string read(size_t len)
    {
        ASSERT(len <= bytesReadable());
        size_t readLen = (len <= bytesReadable()) ? len : bytesReadable();
        string res{peek(), readLen};
        resume(len);
        return res;
    }
    string readAll()
    {
        return read(bytesReadable());
    }

    void append(const char *data, size_t len);
    void append(const void *data, size_t len)
    {
        append(static_cast<const char *>(data), len);
    }

    void writeInt8(int8_t val)
    {
        append(&val, sizeof(val));
    }
    void writeUInt8(uint8_t val)
    {
        append(&val, sizeof(val));
    }
    void writeInt16(int16_t val)
    {
        writeInteger(val);
    }
    void writeUInt16(uint16_t val)
    {
        writeInteger(val);
    }
    void writeInt32(int32_t val)
    {
        writeInteger(val);
    }
    void writeUInt32(uint32_t val)
    {
        writeInteger(val);
    }
    void writeInt64(int64_t val)
    {
        writeInteger(val);
    }
    void writeUInt64(uint64_t val)
    {
        writeInteger(val);
    }

    int handleSocketRead(int sockFd, int &savedErrno);

private:
    template <typename T>
    void writeInteger(T val)
    {
        static_assert(std::is_integral<T>::value, "writeInteger() need an integer type input!");
        static_assert(sizeof(T) > 1, "writeInteger() need an input whose size is bigger than 1 Byte");
        auto netVal = endian::hostToNet(val);
        append(&netVal, sizeof(netVal));
    }
    // make more space if necessary
    void makeMoreSpace(size_t len);
    std::vector<char> data_;
    size_t readIdx_ = PREPEND_SIZE;
    size_t writeIdx_ = PREPEND_SIZE;
};
} // namespace conn
} // namespace wind
