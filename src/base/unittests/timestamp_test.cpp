//
// Created by tracy on 2020/9/21.
//

#include "base/TimeStamp.h"

#include <iostream>

void testTimeStampToString() {
    wind::TimeStamp now = wind::TimeStamp::now();
    std::cout << now.toString() << std::endl; // default case
    std::cout << now.toString(wind::TimePrecision::SECOND) << std::endl;
    std::cout << now.toString(wind::TimePrecision::MILLI) << std::endl;
    std::cout << now.toString(wind::TimePrecision::MICRO) << std::endl;
    std::cout << now.toString(wind::TimePrecision::NANO) << std::endl;
}

void testTimeStampToFormattedString() {
    wind::TimeStamp now = wind::TimeStamp::now();
    std::cout << now.toFormattedString() << std::endl; // default case
    std::cout << now.toFormattedString(wind::TimePrecision::SECOND) << std::endl;
    std::cout << now.toFormattedString(wind::TimePrecision::MILLI) << std::endl;
    std::cout << now.toFormattedString(wind::TimePrecision::MICRO) << std::endl;
    std::cout << now.toFormattedString(wind::TimePrecision::NANO) << std::endl;
}

int main() {
    testTimeStampToString();
    testTimeStampToFormattedString();
}
