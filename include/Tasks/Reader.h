//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_READER_H
#define TDR2TREE_READER_H

#include <vector>
#include <memory>
#include <string>

#include "Task.h"
#include "TDRParser.h"

#include <readerwritercircularbuffer.h>


namespace IO {
    class MemoryMap;
}

namespace Task {


    class Reader : public Base {
    private:
        std::vector<std::unique_ptr<IO::MemoryMap>> mapped_files;
        std::vector<std::string> file_names;
        TDR::Parser parser;
        EntryQueue_t output_queue;

    public:
        Reader(const std::vector<std::string> &files, const size_t &capacity = 1024);

        EntryQueue_t &GetQueue(){ return output_queue; }

        void Run() override;

    };

}

#endif //TDR2TREE_READER_H
