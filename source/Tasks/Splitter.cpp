//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Splitter.h"
#include "BasicStruct.h"
#include "timeval.h"

using namespace Task;


Splitter::Splitter(WordQueue_t &input, const double &time_gap, const size_t &cap)
    : input_queue( input ), output_queue( cap ), gap( time_gap ){}


void Splitter::SplitEntries(){
    auto begin = buffer.begin();
    time_val_t pre_time = {begin->timestamp, begin->cfdcorr};
    while ( true ){
        auto end = std::find_if_not(begin, buffer.end(), [&pre_time, this](const word_t &evt){
            time_val_t time = {evt.timestamp, evt.cfdcorr};
            double timediff = time - pre_time;
            pre_time = time;
            return timediff < this->gap;
        });
        if ( end == buffer.end() )
            break;

        output_queue.try_enqueue(std::vector(begin, end));
        begin = end;
    }
    // Then we will remove everything up to begin (but not including)
    buffer.erase(buffer.begin(), begin);
}

void Splitter::Run()
{
    std::vector<word_t> input;
    while ( true ){

        if ( input_queue.wait_dequeue_timed(input, std::chrono::milliseconds(10)) ){
            buffer.insert(buffer.end(), input.begin(), input.end());
            std::sort(buffer.begin(), buffer.end(), [](const word_t &lhs, const word_t &rhs)
            { return (double(rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });
            SplitEntries();
        } else if ( done ){
            output_queue.enqueue(std::move(buffer));
            break;
        } else {
            std::this_thread::yield();
        }
    }
}