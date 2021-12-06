#ifndef WIND_BUFFER_H
#define WIND_BUFFER_H

#include "NonCopyable.h"
#include "Types.h"

namespace wind {
constexpr size_t DEFAULT_BUFFER_SIZE = 4096;

template <size_t N>
class FixedSizeBuffer : NonCopyable {
public:
    FixedSizeBuffer() = default;
    ~FixedSizeBuffer() noexcept = default;

    const char *data() const { return data_; }
    const char *curr() const { return curr_; }
    const char *end() const { return data_ + sizeof(data_); }
    size_t available() const { return static_cast<size_t>(end() - curr()); }
    size_t length() const { return static_cast<size_t>(curr() - data()); }
    constexpr size_t capacity() const { return sizeof(data_); }

    void reset() { curr_ = data_; }
    void clear()
    {
        memZero(data_, sizeof(data_));
        reset();
    }

    void append(const char *s, size_t len)
    {
        size_t bytesToWrite = (available() > len ? len : available());
        ::memcpy(curr_, s, bytesToWrite);
        curr_ += bytesToWrite;
    }
    void append(const string &s) { append(s.c_str(), s.size()); }
    string toString() const { return string(data_, length()); }

private:
    char data_[N];
    char *curr_ = data_;
};

typedef FixedSizeBuffer<DEFAULT_BUFFER_SIZE> DefaultFixedBuffer;
} // namespace wind
#endif // WIND_BUFFER_H