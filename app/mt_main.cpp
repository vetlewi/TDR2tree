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

    // Spin up the worker threads
    std::vector<std::pair<std::thread, Task::Base *>> threads;
    threads.push_back(reader.ConstructThread());
    threads.push_back(converter.ConstructThread());
    threads.push_back(buffer.ConstructThread());
    threads.push_back(splitter.ConstructThread());
    threads.push_back(trigger.ConstructThread());
    for ( auto &sort : sorters ){ threads.push_back(sort.ConstructThread()); }

    // Now we just wait for everything to finish running
    for ( auto &runner : threads ){
        runner.second->Finish();
        if ( runner.first.joinable() ){
            runner.first.join(); // Blocking until task is finished
        }
    }
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

    // Spin up the worker threads
    std::vector<std::pair<std::thread, Task::Base *>> threads;
    threads.push_back(reader.ConstructThread());
    threads.push_back(converter.ConstructThread());
    threads.push_back(buffer.ConstructThread());
    threads.push_back(splitter.ConstructThread());
    threads.push_back(trigger.ConstructThread());
    for ( auto &sort : sorters ){ threads.push_back(sort.ConstructThread()); }

    // Now we just wait for everything to finish running
    for ( auto &runner : threads ){
        runner.second->Finish();
        if ( runner.first.joinable() ){
            runner.first.join(); // Blocking until task is finished
        }
    }
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
    std::vector<std::pair<std::thread, Task::Base *>> threads;
    threads.push_back(reader.ConstructThread());
    threads.push_back(converter.ConstructThread());
    threads.push_back(buffer.ConstructThread());
    threads.push_back(splitter.ConstructThread());
    threads.push_back(trigger.ConstructThread());
    for ( auto &sort : sorters ){ threads.push_back(sort.ConstructThread()); }

    // Now we just wait for everything to finish running
    for ( auto &runner : threads ){
        runner.second->Finish();
        if ( runner.first.joinable() ){
            runner.first.join(); // Blocking until task is finished
        }
    }
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