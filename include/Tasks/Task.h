//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_TASK_H
#define TDR2TREE_TASK_H

#include <atomic>
#include <utility>
#include <thread>
#include <vector>
#include <BasicStruct.h>
#include <Event.h>

#include <atomic_queue.h>
#include <readerwritercircularbuffer.h>
#include <blockingconcurrentqueue.h>

#define USE_ATOMIC_QUEUE

namespace TDR {
    struct Entry_t;
}

//struct word_t;

namespace Task {

    class Base
    {
    protected:
        std::atomic<bool> done = false;
    public:
        virtual ~Base() = default;

        void Finish() { done = true; }
        virtual void Run() = 0;

        std::pair<std::thread, Base *> ConstructThread(){
            return std::make_pair(std::thread(&Base::Run, this), this);
        }
    };

    struct Triggered_event {
        word_t trigger;
        std::vector<word_t> entries;
    };

#ifdef USE_ATOMIC_QUEUE
    typedef std::vector<TDR::Entry_t> entry_buffer_t;
    typedef atomic_queue::AtomicQueueB2<entry_buffer_t, std::allocator<entry_buffer_t>, true, true, true> EntryQueue_t;
#else
    typedef moodycamel::BlockingReaderWriterCircularBuffer<std::vector<TDR::Entry_t>> EntryQueue_t;
#endif // USE_ATOMIC_QUEUE

#ifdef USE_ATOMIC_QUEUE
    typedef std::vector<word_t> word_buffer_t;
    typedef atomic_queue::AtomicQueueB2<word_buffer_t, std::allocator<word_buffer_t>, true, true, true> WordQueue_t;
#else
    typedef moodycamel::BlockingReaderWriterCircularBuffer<std::vector<word_t>> WordQueue_t;
#endif // USE_ATOMIC_QUEUE

    typedef moodycamel::BlockingConcurrentQueue<std::vector<word_t>> MCWordQueue_t;
    typedef moodycamel::BlockingConcurrentQueue<Triggered_event> TEWordQueue_t;
}

#endif //TDR2TREE_TASK_H
