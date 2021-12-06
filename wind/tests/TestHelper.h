//
// Created by tracy on 2021/12/4.
//

#ifndef WIND_TESTHELPER_H
#define WIND_TESTHELPER_H

#include <iostream>

#include <gtest/gtest.h>

namespace wind {
struct TestScopeHelper {
    TestScopeHelper(std::string testSuitName, std::string testName) :
        testSuitName_(testSuitName), testName_(testName)
    {
        std::cout << "-------------------------------------------------------"
                  << std::endl;
        std::cout << "Test " << testSuitName_ << "::" << testName_
                  << " begin:" << std::endl;
    }

    ~TestScopeHelper()
    {
        std::cout << "Test " << testSuitName_ << "::" << testName_ << " end."
                  << std::endl;
        std::cout << "-------------------------------------------------------"
                  << std::endl;
    }

    std::string testSuitName_;
    std::string testName_;
};
} // namespace wind

#define WIND_TEST_BEGIN(testSuitName, testName)                                \
    wind::TestScopeHelper __testScopeHelper(#testSuitName, #testName);

#endif // WIND_TESTHELPER_H
