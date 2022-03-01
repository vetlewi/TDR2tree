//
// Created by Vetle Wegner Ingeberg on 19/04/2021.
//

#include "Event.h"
#include "RootSort.h"

using namespace Task;

RootSort::RootSort(MCWordQueue_t &input, const char *fname, const bool &addback, const bool &_tree)
    : fileManager( fname )
    , histManager( &fileManager )
    , treeManager( &fileManager, "events", "events" )
    , addback_hist( ( addback ) ? fileManager.Mat("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr )
    , tree( _tree )
    , input_queue( input )
{
}

RootSort::RootSort(MCWordQueue_t &input, const char *fname, const CLI::Options &options)
    : fileManager( fname )
    , histManager( &fileManager, options.Trigger.value() )
    , treeManager( &fileManager, "events", "events" )
    , addback_hist( (options.addback.value()) ? fileManager.Mat("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr )
    , tree( options.tree.value() )
    , input_queue( input )
{
}

void RootSort::Run()
{
    std::vector<word_t> input;
    while ( true ){
        if ( input_queue.wait_dequeue_timed(input, std::chrono::microseconds(100)) ){
            Event event(input, addback_hist);
            histManager.AddEntry(event);
            if ( tree )
                treeManager.AddEntry(event);
        } else if ( done ) {
            break;
        } else {
            std::this_thread::yield();
        }
    }
}