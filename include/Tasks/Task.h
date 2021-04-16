//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_TASK_H
#define TDR2TREE_TASK_H

#include <atomic>
#include <utility>
#include <thread>
#include <vector>

#include <readerwritercircularbuffer.h>
#include <blockingconcurrentqueue.h>

namespace TDR {
    struct Entry_t;
}

struct word_t;

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

    typedef moodycamel::BlockingReaderWriterCircularBuffer<std::vector<TDR::Entry_t>> EntryQueue_t;
    typedef moodycamel::BlockingReaderWriterCircularBuffer<std::vector<word_t>> WordQueue_t;
    typedef moodycamel::BlockingConcurrentQueue<std::vector<word_t>> MCWordQueue_t;
}

#endif //TDR2TREE_TASK_H
