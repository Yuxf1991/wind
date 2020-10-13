//
// Created by tracy on 2020/9/20.
//

#include <atomic>
#include <iostream>
#include <string>

#include "base/Condition.h"
#include "base/Thread.h"

namespace wind {
    namespace test {
        class ConditionTest {
        public:
            ConditionTest() : m_thread([this]() { ThreadFunc(); }), m_quit(false) {
                m_thread.start();
            }

            ~ConditionTest() {
                quit();
                if (m_thread.joinable()) {
                    std::cout << "waiting for sub thread done." << std::endl;
                    m_thread.join();
                }
                std::cout << "Quit main thread." << std::endl;
            }

            void quit() {
                if (m_quit) {
                    // do nothing.
                    return;
                }

                m_quit = true;

                LockGuard<Mutex> lock(m_mutex);
                SendMessageLocked("should quit");
            }

            void SendMessageLocked(std::string tmp) {
                assert(CurrentThread::isMainThread());
                assert(m_mutex.isLockedByCurrentThread());

                message = std::move(tmp);

                m_prepared = true;
                m_consumed = false;
                std::cout << "[main thread] send message: " + message << std::endl;
                m_Condition.notify();
            }

            void ConsumeMessageLocked() {
                assert(!CurrentThread::isMainThread());
                assert(m_mutex.isLockedByCurrentThread());

                std::cout << "[sub thread] get message: " + message << std::endl;

                m_prepared = false;
                m_consumed = true;
                m_Condition.notify();
            }

            void ThreadFunc() {
                while (!m_quit) {
                    UniqueLock<Mutex> lock(m_mutex);
                    m_Condition.wait(lock, [this](){ return m_prepared; });
                    ConsumeMessageLocked();
                }

                std::cout << "Quit sub thread." << std::endl;
            }

            void MainThreadWork() {
                while (!m_quit) {
                    {
                        UniqueLock<Mutex> lock(m_mutex);
                        m_Condition.wait(lock, [this]() { return m_consumed; });
                    }

                    std::string tmp;
                    std::cout << "Type message please(q or Q to quit): " << std::endl;
                    std::getline(std::cin, tmp);
                    if (tmp.size() == 1 && (tmp[0] == 'q' || tmp[0] == 'Q')) {
                        quit();
                        break;
                    }

                    {
                        LockGuard<Mutex> lock(m_mutex);
                        SendMessageLocked(tmp);
                    }
                }
            }

        private:
            Condition m_Condition;
            Mutex m_mutex;
            wind::Thread m_thread;
            std::atomic<bool> m_quit = false;

            std::string message GUARDED_BY(m_mutex);
            bool m_prepared GUARDED_BY(m_mutex) = false;
            bool m_consumed GUARDED_BY(m_mutex) = true;
        };
    } // namespace test
} // namespace wind

int main() {
    wind::test::ConditionTest t;
    t.MainThreadWork();
    return 0;
}