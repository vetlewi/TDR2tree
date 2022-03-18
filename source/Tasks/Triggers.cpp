//
// Created by Vetle Wegner Ingeberg on 11/03/2022.
//

#include "Triggers.h"
#include "timeval.h"

using namespace Task;

Trigger_worker::Trigger_worker(MCWordQueue_t &input, TEWordQueue_t &output, const double &time, const DetectorType &trig)
        : input_queue( input )
        , output_queue( output )
        , coincidence_time( time )
        , trigger( trig ){}

void Trigger_worker::Run()
{
    std::vector<word_t> input;
    word_t null_trigger = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if ( trigger == DetectorType::any && coincidence_time <= 0 ){
        while ( true ){
            if ( input_queue.wait_dequeue_timed(input, std::chrono::microseconds(100)) ){
                while ( !output_queue.enqueue(Triggered_event(null_trigger, std::move(input))) ){
                    std::this_thread::yield();
                }
            } else if ( input_queue.done ){
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
                while ( !output_queue.try_enqueue(Triggered_event(*trigger_point, std::vector<word_t>(evt_begin, evt_end))) ) // Waiting and trying to enqueue...
                    std::this_thread::yield();
                trigger_point = std::find_if(trigger_point+1, input.end(), [&triggerT, this](const word_t &word){
                    return (GetDetectorPtr(word.address)->type == this->trigger) &&
                           std::abs(time_val_t({word.timestamp, word.cfdcorr}) - triggerT) > this->coincidence_time;
                });
            }
        } else if ( input_queue.done ){
            break;
        } else {
            std::this_thread::yield();
        }
    }
    output_queue.done = true;
}

Triggers::Triggers(MCWordQueue_t &input, const size_t &workers, const double &time, const DetectorType &trigger, const size_t &cap)
    : input_queue( input )
    , output_queue( cap )
    , triggers( workers )
{
    for ( int n = 0 ; n < workers ; ++n ){
        triggers[n] = new Trigger_worker(input_queue, output_queue, time, trigger);
    }
}

Triggers::~Triggers()
{
    for ( auto trigger : triggers ){
        delete trigger;
    }
}