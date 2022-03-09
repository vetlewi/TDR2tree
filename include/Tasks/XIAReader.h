//
// Created by Vetle Wegner Ingeberg on 09/03/2022.
//

#ifndef XIAREADER_H
#define XIAREADER_H

#include <Task.h>

class ProgressUI;

namespace IO {
    class MemoryMap;
}

namespace Task {

    class XIAReader : public Base {

    private:
        std::vector<std::unique_ptr<IO::MemoryMap>> mapped_files;
        std::vector<std::string> file_names;
        WordQueue_t output_queue;
        ProgressUI *ui;
        const size_t buffer_size;
        std::vector<word_t> buffer;

        void RunWithUI();
        void RunWithoutUI();

        void AddEntries(const char *begin, const char *end);

    public:
        XIAReader(const std::vector<std::string> &files, ProgressUI *ui = nullptr, const size_t &capacity = 1024,
                  const size_t &buffer_size = 16384);

        WordQueue_t &GetQueue(){ return output_queue; }

        void Run() override;

    };

}

#endif // XIAREADER_H
