//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "TDRTypes.h"
#include "XIA_CFD.h"
#include "MemoryMap.h"
#include "ProgressUI.h"
#include "RootWriter.h"
#include "CommandLineInterface.h"

#include <vector>
#include <thread>

#include "Reader.h"
#include "Converter.h"
#include "Buffer.h"
#include "Splitter.h"
#include "Trigger.h"
#include "Sort.h"
#include "RootSort.h"

#include <TFileMerger.h>
#include <TROOT.h>
#include <fmt/format.h>

template<typename T>
class ThreadPool
{
private:
    std::vector<std::pair<T, Task::Base *>> threads;

    void end_all()
    {
        for ( auto &thread : threads ) {
            thread.second->Finish();
        }

        // Then we will try to join all the threads.
        for ( auto &thread : threads ) {
            if ( thread.first.joinable() )
                thread.first.join();
        }

    }

public:
    ThreadPool() = default;

    void AddTask(Task::Base *task)
    {
        threads.push_back(task->ConstructThread());
    }

    void Wait()
    {
        while ( true ) {
            for ( auto &thread : threads ) {
                try {
                    if ( thread.second->check_status() ) {
                        end_all();
                        return;
                    }
                } catch ( std::exception &ex ) {
                    std::cerr << "Got an exception. Aborting." << std::endl;
                    end_all();
                    return;
                }
            }
            std::this_thread::yield();
        }
    }

    void DoEnd()
    {
        end_all();
        try {
            for ( auto &thread : threads )
                thread.second->check_status();
        } catch (std::exception &ex) {
            std::cerr << "Got an exception. Aborting." << std::endl;

        }
    }


};

// Function to extract name before the .root
std::string RemovePost(const std::string &str)
{
    return str.substr(0, str.find_last_of(".root")-4);
}

std::vector<std::string> OutNames(const std::string &base_name, const size_t &num)
{
    std::vector<std::string> outnames;
    for ( int i = 0 ; i < num ; ++i ){
        outnames.push_back(fmt::format("{}_t{}.root", base_name, i));
    }
    return outnames;
}

Task::Converter::VetoAction OptToTask(const CLI::veto_action &action)
{
    switch ( action ) {
        case CLI::veto_action::ignore : return Task::Converter::ignore;
        case CLI::veto_action::keep : return Task::Converter::keep;
        case CLI::veto_action::remove : return Task::Converter::remove;
    }
}

std::vector<std::string> TriggerSort(ProgressUI *ui, const CLI::Options &options)
{
    std::vector<std::string> outnames = OutNames(RemovePost(options.output.value()), 4);
    Task::Reader reader(options.input.value(), ui);
    Task::Converter converter(reader.GetQueue(), OptToTask(options.VetoAction.value()));
    Task::Buffer buffer(converter.GetQueue());
    Task::Splitter splitter(buffer.GetQueue(), options.SplitTime.value());
    Task::Trigger trigger(splitter.GetQueue(), options.coincidenceTime.value(), options.Trigger.value());
    Task::RootSort sorters[] = {
            Task::RootSort(trigger.GetQueue(), outnames[0].c_str(), options),
            Task::RootSort(trigger.GetQueue(), outnames[1].c_str(), options),
            Task::RootSort(trigger.GetQueue(), outnames[2].c_str(), options),
            Task::RootSort(trigger.GetQueue(), outnames[3].c_str(), options)
    };

    ThreadPool<std::thread> pool;
    pool.AddTask(&reader);
    pool.AddTask(&converter);
    pool.AddTask(&buffer);
    pool.AddTask(&splitter);
    pool.AddTask(&trigger);
    for ( auto &sort : sorters ){ pool.AddTask(&sort); }
    pool.Wait();
    return outnames;
}

std::vector<std::string> RFTriggerSort(ProgressUI *ui, const CLI::Options &options)
{
    std::vector<std::string> outnames = OutNames(RemovePost(options.output.value()), 4);
    Task::Reader reader(options.input.value(), ui);
    Task::Converter converter(reader.GetQueue(), OptToTask(options.VetoAction.value()));
    Task::Buffer buffer(converter.GetQueue());
    Task::Splitter splitter(buffer.GetQueue(), options.SplitTime.value());
    Task::Trigger trigger(splitter.GetQueue(), options.coincidenceTime.value(), options.Trigger.value());
    Task::RFRootSort sorters[] = {
            Task::RFRootSort(trigger.GetQueue(), outnames[0].c_str(), options),
            Task::RFRootSort(trigger.GetQueue(), outnames[1].c_str(), options),
            Task::RFRootSort(trigger.GetQueue(), outnames[2].c_str(), options),
            Task::RFRootSort(trigger.GetQueue(), outnames[3].c_str(), options)
    };

    ThreadPool<std::thread> pool;
    pool.AddTask(&reader);
    pool.AddTask(&converter);
    pool.AddTask(&buffer);
    pool.AddTask(&splitter);
    pool.AddTask(&trigger);
    for ( auto &sort : sorters ){ pool.AddTask(&sort); }
    //reader.Run();
    pool.Wait();
    return outnames;
}
template <class SortType>
std::vector<std::string> TriggeredSort(ProgressUI *ui, const CLI::Options &options)
{
    std::vector<std::string> outnames = OutNames(RemovePost(options.output.value()), 4);
    Task::Reader reader(options.input.value(), ui);
    Task::Converter converter(reader.GetQueue(), OptToTask(options.VetoAction.value()));
    Task::Buffer buffer(converter.GetQueue());
    Task::Splitter splitter(buffer.GetQueue(), options.SplitTime.value());
    Task::Trigger trigger(splitter.GetQueue(), options.coincidenceTime.value(), options.Trigger.value());
    SortType sorters[] = {
            SortType(trigger.GetQueue(), outnames[0].c_str(), options),
            SortType(trigger.GetQueue(), outnames[1].c_str(), options),
            SortType(trigger.GetQueue(), outnames[2].c_str(), options),
            SortType(trigger.GetQueue(), outnames[3].c_str(), options)
    };

    // Spin up the worker threads
    ThreadPool<std::thread> pool;
    //pool.AddTask(&reader);
    pool.AddTask(&converter);
    pool.AddTask(&buffer);
    pool.AddTask(&splitter);
    pool.AddTask(&trigger);
    for ( auto &sort : sorters ){ pool.AddTask(&sort); }
    reader.Run();
    //auto prog = ui->FinishSort("");
    pool.DoEnd();
    //prog.Finish();
    return outnames;
}

std::vector<std::string> GapSort(ProgressUI *ui, const CLI::Options &options)
{
    std::vector<std::string> outnames = OutNames(RemovePost(options.output.value()), 4);
    Task::Reader reader(options.input.value(), ui);
    Task::Converter converter(reader.GetQueue(), OptToTask(options.VetoAction.value()));
    Task::Buffer buffer(converter.GetQueue());
    Task::Splitter splitter(buffer.GetQueue(), options.coincidenceTime.value());
    Task::Trigger trigger(splitter.GetQueue(), -1, DetectorType::any);
    Task::RootSort sorters[] = {
            Task::RootSort(trigger.GetQueue(), outnames[0].c_str(), options),
            Task::RootSort(trigger.GetQueue(), outnames[1].c_str(), options),
            Task::RootSort(trigger.GetQueue(), outnames[2].c_str(), options),
            Task::RootSort(trigger.GetQueue(), outnames[3].c_str(), options)
    };

    // Spin up the worker threads
    // Spin up the worker threads
    ThreadPool<std::thread> pool;
    pool.AddTask(&reader);
    pool.AddTask(&converter);
    pool.AddTask(&buffer);
    pool.AddTask(&splitter);
    pool.AddTask(&trigger);
    for ( auto &sort : sorters ){ pool.AddTask(&sort); }
    pool.DoEnd();
    return outnames;
}

int main_func(int argc, char *argv[])
{
    CLI::Options options;
    try {
        options = CLI::ParseCLA(argc, argv);
    } catch ( std::exception &e ){
        return 1; // Error
    }


    ProgressUI progress;
    ROOT::EnableThreadSafety();
    std::vector<std::string> outfiles;
    switch ( options.sortType.value() ) {
        case CLI::sort_type::coincidence :
            if ( options.Trigger.value() == rfchan ){
                outfiles = RFTriggerSort(&progress, options);
            } else if ( options.Trigger.value() == de_sect ){
                outfiles = TriggeredSort<Task::ParticleRootSort>(&progress, options);
            } else {
                outfiles = RFTriggerSort(&progress, options);
            }
            break;
        case CLI::sort_type::gap :
            outfiles = GapSort(&progress, options);
            break;
    }

    // Merge files
    //auto prog = progress.FinishSort(options.output.value());
    ROOT::MergeFiles(outfiles, options.output.value());
    for ( const auto& outfile : outfiles ){
        system(std::string("rm " + outfile).c_str());
    }
    //prog.Finish();
    return 0;
}

int main(int argc, char *argv[])
{
    try {
        return main_func(argc, argv);
    } catch (std::exception &e){
        std::cerr << "Error, got exception '" << e.what() << "'" << std::endl;
        return 1;
    }
}