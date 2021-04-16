//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Trigger.h"
#include "timeval.h"
#include "BasicStruct.h"

using namespace Task;

Trigger::Trigger(MCWordQueue_t &input, const double &time, const DetectorType &trig, const size_t &cap)
    : input_queue( input )
    , output_queue( cap )
    , coincidence_time( time )
    , trigger( trig ){}

/* void Trigger::work()
{
    auto begin = std::find_if(buffer.begin(), buffer.end(), [this](const word_t &evt){
        return GetDetectorPtr(evt.address)->type == this->trigger;
    });

    while ( begin < buffer.end() ){

        begin = std::find_if(buffer.begin(), buffer.end(), [this](const word_t &evt){
            return GetDetectorPtr(evt.address)->type == this->trigger; });
        if ( begin == buffer.end() ){
            break;
        }

        time_val_t trigger_time = {begin->timestamp, begin->cfdcorr};
        auto evt_begin = std::make_reverse_iterator(std::find_if_not(std::make_reverse_iterator(begin),
                                                                     std::make_reverse_iterator(buffer.begin()),
                                           [&trigger_time, this](const word_t &evt){
            return std::abs(time_val_t({evt.timestamp, evt.cfdcorr}) - trigger_time) < this->coincidence_time;
        })) + 1;

        auto evt_end = std::find_if_not(begin, buffer.end(), [&trigger_time, this](const word_t &evt){
            return std::abs(time_val_t({evt.timestamp, evt.cfdcorr}) - trigger_time) < this->coincidence_time;
        });

        if ( evt_end == buffer.end() ){
            break;
            // Delete everything up until the point where our trigger was than leave
            event
        }
    }

    // Once outside the loop we will remove everything in the buffer that has a time difference
}*/

void Trigger::Run()
{
    std::vector<word_t> input;

    while ( true ){
        if ( input_queue.wait_dequeue_timed(input, std::chrono::microseconds(100)) ){

            auto trigger_point = std::find_if(input.begin(), input.end(), [this](const word_t &word){
                return GetDetectorPtr(word.address)->type == this->trigger;
            });

            while ( trigger_point < input.end() ){
                time_val_t triggerT = {trigger_point->timestamp, trigger_point->cfdcorr};

                auto evt_begin = std::find_if(input.begin(), trigger_point, [&triggerT, this](const word_t &word){
                    return std::abs(time_val_t({word.timestamp, word.cfdcorr}) - triggerT) < this->coincidence_time;
                });

                auto evt_end = std::find_if_not(trigger_point, input.end(), [&triggerT, this](const word_t &word){
                    return std::abs(time_val_t({word.timestamp, word.cfdcorr}) - triggerT) < this->coincidence_time;
                });

                output_queue.enqueue({evt_begin, evt_end});
                trigger_point = std::find_if(trigger_point+1, input.end(), [&triggerT, this](const word_t &word){
                    return (GetDetectorPtr(word.address)->type == this->trigger) &&
                           std::abs(time_val_t({word.timestamp, word.cfdcorr}) - triggerT) > this->coincidence_time;
                });
            }
        } else if ( done ){
            break;
        } else {
            std::this_thread::yield();
        }
    }
}