//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Buffer.h"
#include "BasicStruct.h"

using namespace Task;

Buffer::Buffer(WordQueue_t &input, const size_t &buf_size, const size_t &cap)
    : input_queue( input )
    , output_queue( cap )
    , size( buf_size )
{
    buffer.reserve( 2*size );
}

void Buffer::Run()
{
    std::vector<word_t> input;
    while ( true ){
        // First we will check if there is enough data to push to queue
        if ( buffer.size() > size ){
            // We push everything that we can
            output_queue.try_enqueue(std::vector(buffer.begin(), buffer.begin()+buffer.size()-size));
            buffer.erase(buffer.begin(), buffer.begin()+buffer.size()-size);
        }

        if ( input_queue.wait_dequeue_timed(input, std::chrono::milliseconds(10)) ){
            buffer.insert(buffer.end(), input.begin(), input.end());
            std::sort(buffer.begin(), buffer.end(), [](const word_t &lhs, const word_t &rhs)
            { return (double(rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });
        } else if ( done ){
            output_queue.wait_enqueue(std::move(buffer));
            break;
        } else {
            std::this_thread::yield();
        }
    }
}