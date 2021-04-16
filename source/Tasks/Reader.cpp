//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Reader.h"

#include "TDREntry.h"
#include "MemoryMap.h"
#include "ProgressUI.h"

using namespace Task;

extern ProgressUI progress;

Reader::Reader(const std::vector<std::string> &files, const size_t &capacity)
    : file_names( files )
    , output_queue( capacity )
{
    for ( auto &file : files ){
        mapped_files.emplace_back(new IO::MemoryMap(file.c_str()));
    }
}

void Reader::Run()
{
    int num = 0;
    for ( auto &file : mapped_files ){
        progress.StartNewFile(file_names[num], file->GetSize());
        bool last = (num++ == mapped_files.size());
        auto *end = file->GetPtr() + file->GetSize();
        auto *header = TDR::FindHeader(file->GetPtr(), end);
        while ( header < end ){
            auto *next_header = TDR::FindNextHeader(header, end);
            output_queue.wait_enqueue(parser.ParseBuffer(header, last && !(next_header < end)));
            progress.UpdateReadProgress(header - file->GetPtr());
            header = next_header;
        }
    }
}