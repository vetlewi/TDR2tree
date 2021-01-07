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

#if LOG_ENABLED
#include <spdlog/spdlog.h>
#endif // LOG_ENABLED

#if POSTGRESQL_ENABLED
#include <tao/pq.hpp>
#endif // POSTGRESQL_ENABLED

#include <zstr.hpp>


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

void RunSplitterThread(const Settings_t *settings, const bool *running)
{
    try {
        SpliterThread(settings, running);
    } catch (const std::exception &e){
#if LOG_ENABLED
        spdlog::get("console")->error("Splitter thread got and exception {}", e.what());
#endif // LOG_ENABLED
    }
}

// #################################################################

bool Make_events(const Settings_t *settings)
{
    std::vector<Parser::Entry_t> entries;
    std::vector<Parser::Entry_t> event;

    if ( !settings->split_queue->wait_dequeue_timed(entries, std::chrono::seconds(1)) )
        return false;

    if ( settings->trigger_type == any ) {
        settings->built_queue->enqueue(entries);
        return true;
    }

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
            entry = stop;
        } else {
            ++entry;
        }
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

void RunEventBuilder(const Settings_t *settings, const bool *running)
{
    try {
        EventBuilderThread(settings, running);
    } catch (const std::exception &e){
#if LOG_ENABLED
        spdlog::get("console")->error("Event builder thread got and exception {}", e.what());
#endif // LOG_ENABLED
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

// #################################################################
#if ROOT_MT_FLAG
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

void RunRootThread(const Settings_t *settings, const bool *running, ROOT::Experimental::TBufferMerger *fm)
{
    try {
        RFT(settings, running, fm);
    } catch (const std::exception &e){
#if LOG_ENABLED
        spdlog::get("console")->error("ROOT filler thread got and exception {}", e.what());
#endif // LOG_ENABLED
    }
}
#endif // ROOT_MT_FLAG
// #################################################################

void CSVString(const Settings_t *settings, bool *running)
{
    char data[2048];
    Parser::Entry_t entry;

    while ( (*running) ){
        if ( settings->input_queue->wait_dequeue_timed(entry, std::chrono::seconds(1)) ){
            sprintf(data, "%d,%d,%d,%lld,%d,%d", entry.address, entry.adcdata, entry.cfddata, entry.timestamp, entry.cfdfail, entry.finishcode);
            settings->str_queue->enqueue(data);
        }
    }

    while ( settings->input_queue->try_dequeue(entry) ){
        sprintf(data, "%d,%d,%d,%lld,%d,%d", entry.address, entry.adcdata, entry.cfddata, entry.timestamp, entry.cfdfail, entry.finishcode);
        settings->str_queue->enqueue(data);
    }

}

// #################################################################

void ConvertCSV(const Settings_t *settings, bool *running)
{
    try {
        CSVString(settings, running);
    } catch (const std::exception &e){
#if LOG_ENABLED
        spdlog::get("console")->error("CSV converter got an exception {}", e.what());
#endif // LOG_ENABLED
    }
}

// #################################################################

void CSVwrite(const Settings_t *settings, const bool *running)
{
    // Open file and write
    zstr::ofstream output(settings->output_file);
    //std::ofstream output(settings->output_file);
    std::string str;
    output << "address,adcdata,cfddata,timestamp,cfdflag,finishflag";
    while ( (*running) ){
        if ( settings->str_queue->wait_dequeue_timed(str, std::chrono::seconds(1)) ){
            output << "\n" << str;
        }
    }

    while ( settings->str_queue->try_dequeue(str) ){
        output  << "\n" << str;
    }
}

// #################################################################

void WriteCSV(const Settings_t *settings, const bool *running)
{
    try {
        CSVwrite(settings, running);
    } catch (const std::exception &e){
#if LOG_ENABLED
        spdlog::get("console")->error("CSV writer got an exception {}", e.what());
#endif // LOG_ENABLED
    }
}

// #################################################################

void GetEnumType(char *str, const DetectorInfo_t &dinfo)
{
    switch ( dinfo.type ) {
        case rfchan :
            sprintf(str, "rf");
            break;
        case labr_3x8 :
            sprintf(str, "labrL");
            break;
        case labr_2x2_fs :
            sprintf(str, "labrF");
            break;
        case labr_2x2_ss :
            sprintf(str, "labrS");
            break;
        case clover :
            sprintf(str, "clover");
            break;
        case de_ring :
            sprintf(str, "siRing");
            break;
        case de_sect :
            sprintf(str, "siSect");
            break;
        case eDet :
            sprintf(str, "siBack");
            break;
        default :
            sprintf(str, "unknown");
            break;
    }
}

// #################################################################
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
#if POSTGRESQL_ENABLED
void SQLfill(const Settings_t *settings, const bool *running)
{
    const auto conn = tao::pq::connection::create("host=localhost port=5432 dbname=experiment");
    conn->prepare("fill", "INSERT INTO real_data_exp (id, type, timestamp, energy, timecorr) VALUES ($1, $2, $3, $4, $5)");

    Parser::Entry_t entry;
    DetectorInfo_t dinfo;
    char enum_type[1024];
    std::string enum_str;
    uint64_t id = 0;
    std::shared_ptr<tao::pq::transaction> tr;
    while ( (*running) ){
        if ( id == 0 )
            tr = conn->transaction();
        if ( settings->input_queue->wait_dequeue_timed(entry, std::chrono::seconds(1)) ){
            dinfo = GetDetector(entry.address);
            GetEnumType(enum_type, dinfo);
            enum_str = enum_type;
            ++id;
            if ( dinfo.type != clover )
                tr->execute("fill", dinfo.detectorNum, enum_str, entry.timestamp, entry.energy, entry.cfdcorr);
            else
                tr->execute("fill", dinfo.detectorNum*4+dinfo.telNum, enum_str, entry.timestamp, entry.energy, entry.cfdcorr);
        }
        if ( id == 16384 ){
            tr->commit();
            id = 0;
        }
    }
    tr->commit();
    id = 0;
    while ( settings->input_queue->wait_dequeue_timed(entry, std::chrono::seconds(1)) ){
        if ( id == 0 )
            tr = conn->transaction();
        dinfo = GetDetector(entry.address);
        GetEnumType(enum_type, dinfo);
        enum_str = enum_type;
        ++id;
        if ( dinfo.type != clover )
            tr->execute("fill", dinfo.detectorNum, enum_str, entry.timestamp, entry.energy, entry.cfdcorr);
        else
            tr->execute("fill", dinfo.detectorNum*4+dinfo.telNum, enum_str, entry.timestamp, entry.energy, entry.cfdcorr);
        if ( id == 16384 ){
            tr->commit();
            id = 0;
        }
    }
}
#endif // POSTGRESQL_ENABLED
#pragma clang diagnostic pop

#if POSTGRESQL_ENABLED
void RunPG(const Settings_t *settings, const bool *running)
{
    try {
        SQLfill(settings, running);
    } catch (const std::exception &e) {
#if LOG_ENABLED
        spdlog::get("console")->error("ROOT filler thread got and exception {}", e.what());
#endif // LOG_ENABLED
    }
}


void ConvertPostgre(const Settings_t *settings)
{
    // First we will setup all the required file fetchers, etc.
    Fetcher::FileBufferFetcher *bf = new Fetcher::MTFileBufferFetcher(settings->buffer_type);
    const Fetcher::Buffer *buf;
    std::vector<Parser::Entry_t> entries;

    // Setup threads
    bool filler_thread_running = true;

    const auto conn = tao::pq::connection::create("host=localhost port=5432 dbname=experiment");
    conn->execute("DROP TABLE IF EXISTS real_data_exp");
    conn->execute("CREATE TABLE real_data_exp( key SERIAL PRIMARY KEY, id INTEGER, type detType, timestamp BIGINT, energy DOUBLE PRECISION, timecorr DOUBLE PRECISION)");

    std::list<std::thread> filler_thread(4);

    for (auto &thread : filler_thread ){
        thread = std::thread(SQLfill, settings, &filler_thread_running);
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

    filler_thread_running = false;

    std::cout << "Waiting for filler thread to finish...";
    size_t approx_start_size = settings->input_queue->size_approx();
    progress.StartFillingHistograms(approx_start_size);
    while ( settings->input_queue->size_approx() > 1000 ){
        progress.UpdateEventBuildingProgress(approx_start_size - settings->input_queue->size_approx());
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
    std::cout << "Waiting for filler thread to finish...";
    for ( auto &thread : filler_thread ){
        if ( thread.joinable() ) {
            thread.join();
        }
    }
    std::cout << " Done" << std::endl;
}
#endif // POSTGRESQL_ENABLED

void ConvertFilesCSV(const Settings_t *settings)
{
    Fetcher::FileBufferFetcher *bf = new Fetcher::MTFileBufferFetcher(settings->buffer_type);
    const Fetcher::Buffer *buf;
    std::vector<Parser::Entry_t> entries;

    bool converter_running = true;
    bool outputter_running = true;

    std::list<std::thread> converter_threads(settings->num_split_threads);
    std::thread outputter_thread(WriteCSV, settings, &outputter_running);

    for ( auto &thread : converter_threads ){
        thread = std::thread(ConvertCSV, settings, &converter_running);
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

    converter_running = false;

    auto approx_start_size = settings->input_queue->size_approx();
    progress.StartBuildingEvents(approx_start_size);
    size_t current_size;
    while ( settings->input_queue->size_approx() > 1000 ){
        current_size = settings->input_queue->size_approx();
        progress.UpdateTreeFillProgress(approx_start_size - current_size);
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }
    progress.Finish();

    for (auto &thread : converter_threads){
        if ( thread.joinable() )
            thread.join();
    }

    outputter_running = false;

    approx_start_size = settings->str_queue->size_approx();
    progress.StartFillingTree(approx_start_size);
    while ( settings->str_queue->size_approx() > 1000 ){
        current_size = settings->str_queue->size_approx();
        progress.UpdateTreeFillProgress(approx_start_size - current_size);
        std::this_thread::sleep_for(std::chrono::microseconds(250));
    }

    if ( outputter_thread.joinable() )
        outputter_thread.join();

}

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
#if ROOT_MT_FLAG
    std::list<std::thread> fill_threads(settings->num_filler_threads);
#else
    std::list<std::thread> fill_threads(1);
#endif // ROOT_MT_FLAG

    for ( auto &thread : event_threads ){
        thread = std::thread(EventBuilderThread, settings, &builder_running);
    }

    for ( auto &thread : fill_threads ){
#if ROOT_MT_FLAG
        thread = std::thread(RFT, settings, &filler_running, &bufferMerger);
#else
        thread = std::thread(RootFillerThread, settings, &filler_running, -1);
#endif // ROOT_MT_FLAG
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