#ifndef TS_QUEUE_H
#define TS_QUEUE_H
#include "Common.h"

namespace MyFramework {
    template<typename T>
    class ThreadSafeDeque {
        public:
            ThreadSafeDeque() = default;
            virtual ~ThreadSafeDeque() { clear(); };
            ThreadSafeDeque(const ThreadSafeDeque&) = delete;

            void push_back(const T& item) {
                std::scoped_lock lock(muxDeque);
                deque.push_back(item);
            }

            void push_front(const T& item) {
                std::scoped_lock lock(muxDeque);
                deque.push_front(item);
            }

            const T& front() {
                std::scoped_lock lock(muxDeque);
                return deque.front();
            }

            const T& back() {
                std::scoped_lock lock(muxDeque);
                return deque.back();
            }

            T pop_front() {
                std::scoped_lock lock(muxDeque);
                auto front = std::move(deque.front());
                deque.pop_front();
                return front;
            }

            T pop_back() {
                std::scoped_lock lock(muxDeque);
                auto back = std::move(deque.back());
                deque.pop_back();
                return back;
            }

            bool empty() {
                std::scoped_lock lock(muxDeque);
                return deque.empty();
            }

            size_t count() {
                std::scoped_lock lock(muxDeque);
                return deque.size();
            }

            void clear() {
                std::scoped_lock lock(muxDeque);
                deque.clear();
            }

        protected:
            std::mutex muxDeque;
            std::deque<T> deque;
    };
}
#endif