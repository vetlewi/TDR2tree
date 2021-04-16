//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_CONVERTER_H
#define TDR2TREE_CONVERTER_H

#include "Task.h"

#include <vector>

#include <readerwritercircularbuffer.h>


namespace Task {

    class Converter : public Base
    {
    public:
        enum VetoAction {
            ignore,
            keep,
            remove
        };

    private:
        VetoAction vetoAction;
        EntryQueue_t &input_queue;
        WordQueue_t output_queue;

        std::vector<word_t> TDRtoWord(const std::vector<TDR::Entry_t> &entries);

    public:

        Converter(EntryQueue_t &input, const VetoAction &action = ignore, const size_t &cap = 128)
            : vetoAction( action ), input_queue( input ), output_queue( cap ){}

        WordQueue_t &GetQueue(){ return output_queue; }

        void Run() override;

    };

}

#endif //TDR2TREE_CONVERTER_H
