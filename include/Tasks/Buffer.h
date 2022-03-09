//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_BUFFER_H
#define TDR2TREE_BUFFER_H

#include "Task.h"


namespace Task {

    class Buffer : public Base
    {

    private:
        WordQueue_t &input_queue;
        WordQueue_t output_queue;

        const size_t size;
        std::vector<word_t> buffer;

    public:

        Buffer(WordQueue_t &input, const size_t &buf_size = 196608, const size_t &cap = 1024);
        WordQueue_t &GetQueue(){ return output_queue; }
        void Run() override;

    };

}

#endif //TDR2TREE_BUFFER_H
