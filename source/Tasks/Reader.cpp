//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Reader.h"

#include "TDREntry.h"
#include "MemoryMap.h"
#include "ProgressUI.h"

using namespace Task;

Reader::Reader(const std::vector<std::string> &files, ProgressUI *_ui, const size_t &capacity)
    : file_names( files )
    , output_queue( capacity )
    , ui( _ui )
{
    for ( auto &file : files ){
        mapped_files.emplace_back(new IO::MemoryMap(file.c_str()));
    }
}

void Reader::RunWithUI()
{
    int num = 0;
    for ( auto &file : mapped_files ){
        ProgressBar bar = ui->StartNewFile(file_names[num], file->GetSize());
        bool last = (num++ == mapped_files.size());
        auto *end = file->GetPtr() + file->GetSize();
        auto *header = TDR::FindHeader(file->GetPtr(), end);
        while ( header < end ){
            auto *next_header = TDR::FindNextHeader(header, end);
#ifdef USE_ATOMIC_QUEUE
            output_queue.push(parser.ParseBuffer(header, last && !(next_header < end)));
#else
            output_queue.wait_enqueue(parser.ParseBuffer(header, last && !(next_header < end)));
#endif // USE_ATOMIC_QUEUE
            bar.UpdateProgress(header - file->GetPtr());
            header = next_header;
        }
        bar.FinishProgress();
    }
}

void Reader::RunWithoutUI()
{
    int num = 0;
    for ( auto &file : mapped_files ){
        bool last = (num++ == mapped_files.size());
        auto *end = file->GetPtr() + file->GetSize();
        auto *header = TDR::FindHeader(file->GetPtr(), end);
        while ( header < end ){
            auto *next_header = TDR::FindNextHeader(header, end);
#ifdef USE_ATOMIC_QUEUE
            output_queue.push(parser.ParseBuffer(header, last && !(next_header < end)));
#else
            output_queue.wait_enqueue(parser.ParseBuffer(header, last && !(next_header < end)));
#endif // USE_ATOMIC_QUEUE
            header = next_header;
        }
    }
}

void Reader::Run()
{
    if ( ui )
        RunWithUI();
    else
        RunWithoutUI();

    // Once done, we will just
    output_queue.done = true;
}