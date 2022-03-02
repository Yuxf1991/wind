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

#ifndef WIND_TESTS_TESTHELPER_H
#define WIND_TESTS_TESTHELPER_H

#include <iostream>

#include <gtest/gtest.h>

namespace wind {
struct TestScopeHelper {
    TestScopeHelper(std::string testSuitName, std::string testName) : testSuitName_(testSuitName), testName_(testName)
    {
        std::cout << "-------------------------------------------------------" << std::endl;
        std::cout << "Test " << testSuitName_ << "::" << testName_ << " begin:" << std::endl;
    }

    ~TestScopeHelper()
    {
        std::cout << "Test " << testSuitName_ << "::" << testName_ << " end." << std::endl;
        std::cout << "-------------------------------------------------------" << std::endl;
    }

    std::string testSuitName_;
    std::string testName_;
};
} // namespace wind

#define WIND_TEST_BEGIN(testSuitName, testName) wind::TestScopeHelper __testScopeHelper(#testSuitName, #testName);

#endif // WIND_TESTS_TESTHELPER_H
