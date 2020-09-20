//
// Created by tracy on 2020/9/20.
//

#include <atomic>
#include <iostream>
#include <thread>
#include <string>

#include "base/Condition.h"

namespace wind {
    namespace test {
        class ConditionTest {
        public:
            ConditionTest() : m_quit(false) {
                m_thread = std::thread{[this]() { ThreadFunc(); }};
            }

            ~ConditionTest() {
                quit();
                if (m_thread.joinable()) {
                    std::cout << "waiting for sub thread done." << std::endl;
                    m_thread.join();
                }
            }

            void quit() {
                if (m_quit) {
                    // do nothing.
                    return;
                }

                m_quit = true;

                LockGuard<Mutex> lock(m_mutex);
                message = "should quit";
                m_prepared = true;
                std::cout << "send message: " + message << std::endl;
                m_Condition.notify();
            }

            void ThreadFunc() {
                while (!m_quit) {
                    UniqueLock<Mutex> lock(m_mutex);
                    m_Condition.wait(lock, [this](){ return m_prepared; });
                    std::cout << "get message: " + message << std::endl;
                    m_prepared = false;
                }

                std::cout << "Quit sub thread." << std::endl;
            }

            void MainThreadWork() {
                while (!m_quit) {
                    std::string tmp;
                    std::getline(std::cin, tmp);
                    if (tmp.size() == 1 && (tmp[0] == 'q' || tmp[0] == 'Q')) {
                        std::cout << "Quit main thread." << std::endl;
                        quit();
                        break;
                    }

                    {
                        LockGuard<Mutex> lock(m_mutex);
                        message = tmp;
                        m_prepared = true;
                        std::cout << "send message: " + message << std::endl;
                        m_Condition.notify();
                    }
                }
            }

        private:
            Condition m_Condition;
            Mutex m_mutex;
            // TODO: use our own thread;
            std::thread m_thread;
            std::atomic<bool> m_quit = false;

            std::string message GUARDED_BY(m_mutex);
            bool m_prepared GUARDED_BY(m_mutex) = false;
        };
    } // namespace test
} // namespace wind

int main() {
    wind::test::ConditionTest t;
    t.MainThreadWork();
    return 0;
}