//
// Created by Vetle Wegner Ingeberg on 12/11/2019.
//

#include "SortUtillities.h"

// C++ STD headers
#include <list>
#include <thread>
#include <iostream>
#include <mutex>

// C headers
#include <cmath>

// Unix headers
#include <unistd.h>
#include <sys/wait.h>

// Buffer library
#include <Buffer/MTFileBufferFetcher.h>

// Parser library
#include <Parser/Parser.h>

// Param library
#include <Parameters/experimentsetup.h>

// Event library
#include <Event/Event.h>

// ROOT interface library
#include <RootInterface/RootFileManager.h>
#include <RootInterface/HistManager.h>
#include <RootInterface/TreeManager.h>

// Utillities
#include <Utilities/ProgressUI.h>
#include <Utilities/CLI_interface.h>

// ROOT headers
#include <ROOT/TBufferMerger.hxx>


extern ProgressUI progress;

inline double TimeDiff(const Parser::Entry_t &lhs, const Parser::Entry_t &rhs)
{
    return double(lhs.timestamp - rhs.timestamp) + (lhs.cfdcorr - rhs.cfdcorr);
}

// #################################################################

bool Split_entries(const Settings_t *settings, std::vector<Parser::Entry_t> &entries)
{
    Parser::Entry_t entry;
    if ( !settings->input_queue->wait_dequeue_timed(entry, std::chrono::seconds(1)) )
        return false;

    if ( entries.empty() ) {
        entries.push_back(entry);
    }

    if ( fabs(TimeDiff(entry, entries.back())) < settings->split_time ){
        entries.push_back(entry);
    } else {
        settings->split_queue->enqueue(entries);
        entries.clear();
        entries.push_back(entry);
    }
    return true;
}

// #################################################################

void SpliterThread(const Settings_t *settings, const bool *running)
{
    std::vector<Parser::Entry_t> entries;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
    while (*running){

        if ( !Split_entries(settings, entries) ){
            // Do nothing...
        }

    }
#pragma clang diagnostic pop

    while ( Split_entries(settings, entries) ){
        // Everyting done in function...
    }
}

// #################################################################

bool Make_events(const Settings_t *settings)
{
    std::vector<Parser::Entry_t> entries;
    std::vector<Parser::Entry_t> event;

    if ( !settings->split_queue->wait_dequeue_timed(entries, std::chrono::seconds(1)) )
        return false;

    auto entry = std::begin(entries);

    while ( entry != std::end(entries) ){

        if ( GetDetectorType((*entry).address) == settings->trigger_type ){
            auto start = entry;
            while ( start != std::begin(entries) ){
                if ( fabs(TimeDiff(*start, *entry)) < settings->event_time ){
                    --start;
                } else
                    break;
            }

            auto stop = entry+1;
            while ( stop != std::end(entries) ){
                if ( fabs(TimeDiff(*stop, *entry)) < settings->event_time ){
                    ++stop;
                } else
                    break;
            }
            settings->built_queue->enqueue(std::vector<Parser::Entry_t>(start, stop));
        }
        ++entry;
    }
    return true;
}

// #################################################################

void EventBuilderThread(const Settings_t *settings, const bool *running)
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
    while ( (*running) ){
        if ( !Make_events(settings) ) {
            // Do nothing
        }
    }
#pragma clang diagnostic pop

    while ( Make_events(settings) ){
        // Everything done in function...
    }
}

// #################################################################

void RootFillerThread(const Settings_t *settings, const bool *running, const int thread_id)
{
    char tmp[1024];
    if ( thread_id >= 0 )
        sprintf(tmp, "%s_%i", settings->output_file.c_str(), thread_id);
    else
        sprintf(tmp, "%s", settings->output_file.c_str());
    RootFileManager fileManager(tmp, "RECREATE", settings->file_title.c_str());
    HistManager histManager(&fileManager);
    TreeManager treeManager(&fileManager,
            settings->tree_name.c_str(),
            settings->tree_title.c_str(),
            settings->event_type->New());


    std::vector<Parser::Entry_t> event;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
    while ( (*running) ){

        if ( settings->built_queue->wait_dequeue_timed(event, std::chrono::seconds(1)) ){
            Event::iThembaEvent evt(event);
            histManager.AddEntry(evt);
            if ( settings->build_tree )
                treeManager.AddEntry(&evt);
        }
    }
#pragma clang diagnostic pop

    while ( settings->built_queue->try_dequeue(event) ){
        Event::iThembaEvent evt(event);
        histManager.AddEntry(evt);
        if ( settings->build_tree )
            treeManager.AddEntry(&evt);
    }
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
void RFT(const Settings_t *settings, const bool *running, ROOT::Experimental::TBufferMerger *fm)
{
    RootMergeFileManager fileManager(fm);
    HistManager histManager(&fileManager);
    TreeManager treeManager(&fileManager,
                            settings->tree_name.c_str(),
                            settings->tree_title.c_str(),
                            settings->event_type->New());


    std::vector<Parser::Entry_t> event;

    while ( (*running) ){

        if ( settings->built_queue->wait_dequeue_timed(event, std::chrono::seconds(1)) ){
            Event::iThembaEvent evt(event);
            histManager.AddEntry(evt);
            if ( settings->build_tree )
                treeManager.AddEntry(&evt);
        }
    }

    while ( settings->built_queue->try_dequeue(event) ){
        Event::iThembaEvent evt(event);
        histManager.AddEntry(evt);
        if ( settings->build_tree )
            treeManager.AddEntry(&evt);
    }
}
#pragma clang diagnostic pop


// #################################################################

void ConvertFiles(const Settings_t *settings)
{
    // First we will setup all the required file fetchers, etc.
    Fetcher::FileBufferFetcher *bf = new Fetcher::MTFileBufferFetcher(settings->buffer_type);
    const Fetcher::Buffer *buf;
    std::vector<Parser::Entry_t> entries;

    // Setup threads
    bool splitter_running = true;
    bool builder_running = true;
    bool filler_running = true;

    ROOT::Experimental::TBufferMerger bufferMerger(settings->output_file.c_str(), "RECREATE");

    std::thread split_thread(SpliterThread, settings, &splitter_running);
    std::list<std::thread> event_threads(settings->num_split_threads);
    std::list<std::thread> fill_threads(settings->num_filler_threads);

    for ( auto &thread : event_threads ){
        thread = std::thread(EventBuilderThread, settings, &builder_running);
    }

    int thread_ID = ( settings->num_filler_threads > 1 ) ? 0 : -1;
    for ( auto &thread : fill_threads ){
        thread = std::thread(RFT, settings, &filler_running, &bufferMerger);
    }


    for ( auto &file : settings->input_files ){
        Fetcher::BufferFetcher::Status status = bf->Open(file.c_str(), 0);

        while ( true ){
            buf = bf->Next(status);
            if ( status != Fetcher::BufferFetcher::OKAY ){
                break;
            }
            entries = settings->parser->GetEntry(buf);
            settings->input_queue->enqueue_bulk(std::begin(entries), entries.size());
        }
    }

    // Finish up all the data left

    splitter_running = false;

    std::cout << "Waiting for splitter thread to finish...";
    if ( split_thread.joinable() )
        split_thread.join();
    std::cout << " Done" << std::endl;

    builder_running = false;

    size_t approx_start_size = settings->split_queue->size_approx();
    progress.StartBuildingEvents(approx_start_size);
    while ( settings->split_queue->size_approx() > 1000 ){
        progress.UpdateEventBuildingProgress(approx_start_size - settings->split_queue->size_approx());
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
    progress.Finish();
    std::cout << "Waiting for event builder threads to finish...";
    for ( auto &thread : event_threads ){
        if ( thread.joinable() ) {
            thread.join();
        }
    }
    std::cout << " Done" << std::endl;

    filler_running = false;

    approx_start_size = settings->built_queue->size_approx();
    progress.StartFillingTree(approx_start_size);
    size_t current_size;
    while ( settings->built_queue->size_approx() > 1000 ){
        current_size = settings->built_queue->size_approx();
        progress.UpdateTreeFillProgress(approx_start_size - current_size);
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
    progress.Finish();

    std::cout << "Waiting for filler thread to finish...";
    for ( auto &thread : fill_threads ){
        if ( thread.joinable() ){
            thread.join();
        }
    }
    std::cout << " Done" << std::endl;
}