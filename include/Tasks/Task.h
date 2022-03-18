//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_TASK_H
#define TDR2TREE_TASK_H

#include <atomic>
#include <utility>
#include <thread>
#ifndef __cpp_lib_jthread
    #include <stop_token.hpp>
    #include <jthread.hpp>
#endif // __cpp_lib_jthread
#include <vector>
#include <BasicStruct.h>
#include <Event.h>

#include <atomic_queue.h>
#include <readerwritercircularbuffer.h>
#include <blockingconcurrentqueue.h>

//#define USE_ATOMIC_QUEUE

namespace TDR {
    struct Entry_t;
}

//struct word_t;

namespace Task {

    class Base
    {
    protected:
        std::atomic<bool> is_done = false;
        //std::stop_token done;
        std::atomic<bool> done = false;
        std::optional<std::exception> exception;
    public:
        virtual ~Base() = default;

        void Finish() { done = true; }
        virtual void Run() = 0;

        // Check if an exception was thrown
        bool check_status(){
            if ( exception.has_value() ){
                throw exception.value();
            }
            return is_done;
        }

        void DoRun()
        {
            try {
                this->Run();
            } catch ( const std::exception &ex ){
                std::cerr << "Task " << typeid(*this).name() << " got exception '" << ex.what() << "'" << std::endl;
                exception = ex;
            }
            is_done = true;
        }

        /*std::jthread ConstructThread()
        {
            return std::jthread([&, this](std::stop_token st){
                this->done = std::move(st);
                this->DoRun();
            });
        }*/

        std::pair<std::thread, Base *> ConstructThread()
        {
            return std::make_pair(std::thread(&Base::Run, this), this);
        }
    };

    struct Triggered_event {
        word_t trigger;
        std::vector<word_t> entries;
        Triggered_event() = default;
        Triggered_event(const word_t &_trigger, std::vector<word_t> &&_entries) : trigger( _trigger ), entries( _entries ){}
    };

    template<typename T>
    struct QueueWrapper
    {
        T queue;
        std::atomic<bool> done;

        template<typename U>
        inline bool try_enqueue(U const &u){ return queue.try_enqueue(u); }

        template<typename U>
        inline bool try_enqueue(U &&u){ return queue.try_enqueue(u); }

        template<typename U>
        inline auto enqueue(U const &u){ return queue.enqueue(u); }

        template<typename U>
        inline auto enqueue(U &&u){ return queue.enqueue(u); }

        template<typename U>
        inline void wait_enqueue(U const &u){ queue.wait_enqueue(u); }

        template<typename U>
        inline void wait_enqueue(U &&u){ queue.wait_enqueue(u); }

        template<typename U, typename V>
        inline bool wait_dequeue_timed(U &u, V const &v){ return queue.wait_dequeue_timed(u, v); }

        QueueWrapper(const size_t &capacity) : queue( capacity ), done( false ){}
    };

    using entry_buffer_t = std::vector<TDR::Entry_t>;
    using word_buffer_t = std::vector<word_t>;
#ifdef USE_ATOMIC_QUEUE
    typedef atomic_queue::AtomicQueueB2<entry_buffer_t, std::allocator<entry_buffer_t>, true, true, true> EntryQueue_t;
#else
    //typedef moodycamel::BlockingReaderWriterCircularBuffer<std::vector<TDR::Entry_t>> EntryQueue_t;
    using EntryQueue_t = QueueWrapper<moodycamel::BlockingReaderWriterCircularBuffer<entry_buffer_t>>;
#endif // USE_ATOMIC_QUEUE

#ifdef USE_ATOMIC_QUEUE
    typedef std::vector<word_t> word_buffer_t;
    typedef atomic_queue::AtomicQueueB2<word_buffer_t, std::allocator<word_buffer_t>, true, true, true> WordQueue_t;
#else
    //typedef moodycamel::BlockingReaderWriterCircularBuffer<word_buffer_t> WordQueue_t;
    using WordQueue_t = QueueWrapper<moodycamel::BlockingReaderWriterCircularBuffer<word_buffer_t>>;
#endif // USE_ATOMIC_QUEUE

    //typedef moodycamel::BlockingConcurrentQueue<word_buffer_t> MCWordQueue_t;
    using MCWordQueue_t = QueueWrapper<moodycamel::BlockingConcurrentQueue<word_buffer_t>>;
    //typedef moodycamel::BlockingConcurrentQueue<Triggered_event> TEWordQueue_t;
    using TEWordQueue_t = QueueWrapper<moodycamel::BlockingConcurrentQueue<Triggered_event>>;
}

#endif //TDR2TREE_TASK_H
