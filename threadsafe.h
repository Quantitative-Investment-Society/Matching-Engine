#ifndef TSAFE
#define TSAFE

#include <queue>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <optional>
#include <vector>

namespace ThreadSafe {
    template<typename T, typename _CONTAINER, typename _CMP>
    class priority_queue {
        public:
            ThreadSafe();

            void push(T t) {
                std::scoped_lock lock{access_mutex};
                if (pq.empty()) {
                    pop_cond.notify_one();
                }
                pq.push(t);
            }

            std::optional<T> pop() {
                std::scoped_lock lock{access_mutex};
                if (pq.empty()) {
                    return std::nullopt;
                }
                T val{pq.top()};
                pq.pop();
                return val;
            }

            std::optional<T> await_pop() {
                std::unique_lock lock{access_mutex};
                pop_cond.await(lock, [this](){return !pq.empty() || stop_running});
            }

            std::optional<T> top() {
                std::scoped_lock lock{access_mutex};
                if (pq.empty()) {
                    return std::nullopt;
                }
                return pq.top();
            }

            bool empty() {
                std::scoped_lock lock{access_mutex};
                return pq.empty();
            }

            void stop() {
                stop_running.store(true);
                pop_cond.notify_all();
            }

            size_t size() {
                std::scoped_lock lock{access_mutex};
                return pq.size();
            }


        private:
            std::mutex access_mutex;
            std::priority_queue<T, _CONTAINER, _CMP> pq;
            std::conditional_variable pop_cond;
            std::atomic_bool stop_running{false};
    };


}


#endif