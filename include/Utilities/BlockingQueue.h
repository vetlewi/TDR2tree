//
// Created by Vetle Wegner Ingeberg on 13/01/2021.
//

#ifndef TDR2TREE_BLOCKINGQUEUE_H
#define TDR2TREE_BLOCKINGQUEUE_H

#include <mutex>
#include <condition_variable>
#include <chrono>

#include <queue>

#define MAX_CAPACITY 0x10000

template<typename T>
class BlockingQueue
{
private:

    std::mutex mutex;
    std::condition_variable full_;
    std::condition_variable empty_;
    std::queue<T> queue;

    size_t capacity;

public:

    BlockingQueue(const size_t &cap = MAX_CAPACITY)
        : mutex(), full_(), empty_(), queue(), capacity( cap ){}

    void push(const T &data)
    {
        std::unique_lock<std::mutex> lock(mutex);
        while (queue.size() == capacity)
            full_.wait(lock);

        queue.push(data);
        empty_.notify_all();
    }

    template< class Rep, class Period >
    bool wait_push(const T &data, const std::chrono::duration<Rep, Period> &duration)
    {
        std::unique_lock<std::mutex> lock(mutex);
        while (queue.size() == capacity) {
            if ( full_.wait_for(lock, duration) == std::cv_status::timeout )
                return false;
        }

        queue.push(data);
        empty_.notify_all();
        return true;
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex);
        while ( queue.empty() ){
            empty_.wait( lock );
        }

        T front(queue.front());
        queue.pop();
        full_.notify_all();
        return front;
    }

    template< class Rep, class Period >
    bool wait_pop(T &item, const std::chrono::duration<Rep, Period> &duration)
    {
        std::unique_lock<std::mutex> lock(mutex);
        while ( queue.empty() ){
            if ( empty_.wait_for(lock, duration) == std::cv_status::timeout )
                return false;
        }
        item = queue.front();
        queue.pop();
        full_.notify_all();
        return true;
    }

    size_t size()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    bool enqueue(T const &item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if ( queue.size() == capacity )
            return false;

        queue.push(item);
        empty_.notify_all();
        return true;
    }

    bool try_dequeue(T &item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if ( queue.empty() )
            return false;

        item = queue.front();
        queue.pop();
        full_.notify_all();
        return true;
    }

    template<typename Rep, typename Period>
    inline bool wait_dequeue_timed(T &item, const std::chrono::duration<Rep, Period> &duration)
    {
        return wait_pop(item, duration);
    }

    inline size_t size_approx() { return size(); }

private:

    //DISABLE_COPY_AND_ASSIGN(BlockingQueue);
    BlockingQueue(const BlockingQueue& rhs);
    BlockingQueue& operator= (const BlockingQueue& rhs);

};

#endif //TDR2TREE_BLOCKINGQUEUE_H
