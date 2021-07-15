#include "TimeStamp.h"
#include <gtest/gtest.h>
#include <iostream>

using std::cout;
using std::endl;

TEST(TimeStampTest, NowTest) {
    auto t = wind::TimeStamp::now();
    cout << "nanoSecondsSinceEpoch: " << t.get() << endl;
    cout << "toString: " << t.toString() << endl;
    cout << "toFormattedString: " << t.toFormattedString() << endl;
    EXPECT_GT(t.get(), 0);
}
