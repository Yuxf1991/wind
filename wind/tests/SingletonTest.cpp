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

#include "TestHelper.h"
#include "Singleton.h"
#include "TimeStamp.h"

#include <thread>
#include <vector>
#include <gtest/gtest.h>

namespace wind {
class BookStore : public Singleton<BookStore> {
    DECLARE_SINGLETON(BookStore);

public:
    ~BookStore() noexcept = default;
    size_t bookCnt() const { return books_.size(); }
    void addBook(string book) { books_.push_back(std::move(book)); }
    void removeBook() { books_.pop_back(); }

private:
    BookStore() = default;
    std::vector<string> books_;
};

TEST(SingletonTest, normalTest)
{
    WIND_TEST_BEGIN(SingletonTest, normalTest);
    auto bookStore1 = BookStore::instance();
    auto bookStore2 = BookStore::instance();
    if (bookStore1 == nullptr || bookStore2 == nullptr) {
        std::cout << "Error: can't get book store instance!" << std::endl;
        return;
    }
    EXPECT_EQ(bookStore1, bookStore2);

    std::cout << "bookStore1 bookCnt: " << bookStore1->bookCnt() << std::endl;
    std::cout << "bookStore2 bookCnt: " << bookStore2->bookCnt() << std::endl;
    EXPECT_EQ(bookStore1->bookCnt(), bookStore2->bookCnt());

    string book1("Hello world!");
    std::cout << "Add book " << book1 << "to book store1.\n";
    bookStore1->addBook(book1);
    std::cout << "bookStore1 bookCnt: " << bookStore1->bookCnt() << std::endl;
    std::cout << "bookStore2 bookCnt: " << bookStore2->bookCnt() << std::endl;
    EXPECT_EQ(bookStore1->bookCnt(), bookStore2->bookCnt());

    string book2("Hello world 2!");
    std::cout << "Add book " << book2 << "to book store2.\n";
    bookStore2->addBook(book2);
    std::cout << "bookStore1 bookCnt: " << bookStore1->bookCnt() << std::endl;
    std::cout << "bookStore2 bookCnt: " << bookStore2->bookCnt() << std::endl;
    EXPECT_EQ(bookStore1->bookCnt(), bookStore2->bookCnt());
}

void GetBookStoreTest(const std::shared_ptr<BookStore> &bookStore)
{
    EXPECT_EQ(bookStore, BookStore::instance());
}

TEST(SingletonTest, multiThreadTest)
{
    WIND_TEST_BEGIN(SingletonTest, multiThreadTest);

    for (auto i = 0; i < 100000; ++i) {
        auto t = std::thread{[&]() { GetBookStoreTest(BookStore::instance()); }};
        t.detach();
    }
}
} // namespace wind
