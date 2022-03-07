//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Trigger.h"
#include "timeval.h"
#include "BasicStruct.h"

#include <stdexcept>

using namespace Task;

void Base::Run() {
    throw std::runtime_error("Not implemented");
}

Trigger::Trigger(MCWordQueue_t &input, const double &time, const DetectorType &trig, const size_t &cap)
    : input_queue( input )
    , output_queue( cap )
    , coincidence_time( time )
    , trigger( trig ){}

void Trigger::Run()
{
    std::vector<word_t> input;
    word_t null_trigger = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if ( trigger == DetectorType::any && coincidence_time <= 0 ){
        while ( true ){
            if ( input_queue.wait_dequeue_timed(input, std::chrono::microseconds(100)) ){
                while ( !output_queue.enqueue({null_trigger, input}) ){
                    std::this_thread::yield();
                }
            } else if ( done ){
                return; // At this point we can return
                break;
            } else {
                std::this_thread::yield();
            }
        }
    }

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
                while ( !output_queue.try_enqueue({*trigger_point, std::vector<word_t>(evt_begin, evt_end)}) ) // Waiting and trying to enqueue...
                    std::this_thread::yield();
                trigger_point = std::find_if(trigger_point+1, input.end(), [&triggerT, this](const word_t &word){
                    return (GetDetectorPtr(word.address)->type == this->trigger) &&
                           std::abs(time_val_t({word.timestamp, word.cfdcorr}) - triggerT) > this->coincidence_time;
                });
            }
        } else if ( done ){
            return; // at this point we have nothing left to do in this function.
            break;
        } else {
            std::this_thread::yield();
        }
    }
}