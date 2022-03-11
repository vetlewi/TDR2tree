//
// Created by Vetle Wegner Ingeberg on 11/03/2022.
//

#include <iostream>
#include <exception>

#include <Task.h>
#include <Tasks.h>
#include <Reader.h>
#include <Converter.h>
#include <Buffer.h>
#include <Splitter.h>
#include <Trigger.h>
#include <Triggers.h>
#include <Sort.h>
#include <RootWriter.h>

#include <CommandLineInterface.h>

std::string RemovePost(const std::string &str)
{
    return str.substr(0, str.find_last_of(".root")-4);
}

Task::Converter::VetoAction OptToTask(const CLI::veto_action &action)
{
    switch ( action ) {
        case CLI::veto_action::ignore : return Task::Converter::ignore;
        case CLI::veto_action::keep : return Task::Converter::keep;
        case CLI::veto_action::remove : return Task::Converter::remove;
    }
}

void TriggerSort_stt(ProgressUI *ui, const CLI::Options &options)
{
    Task::Reader reader(options.input.value(), ui);
    Task::Converter converter(reader.GetQueue(), OptToTask(options.VetoAction.value()));
    Task::Buffer buffer(converter.GetQueue());
    Task::Splitter splitter(buffer.GetQueue(), options.coincidenceTime.value());

    Task::Trigger trigger(splitter.GetQueue(), options.coincidenceTime.value(), options.Trigger.value());
    Task::Sorters sorters(trigger.GetQueue(), options);

    // Spin up the worker threads
    std::vector<std::pair<std::thread, Task::Base *>> threads;
    threads.push_back(reader.ConstructThread());
    threads.push_back(converter.ConstructThread());
    threads.push_back(buffer.ConstructThread());
    threads.push_back(splitter.ConstructThread());
    threads.push_back(trigger.ConstructThread());
    for ( auto &sort : sorters ){ threads.push_back(sort->ConstructThread()); }

    // Now we just wait for everything to finish running
    for ( auto &runner : threads ){
        runner.second->Finish();
        if ( runner.first.joinable() ){
            runner.first.join(); // Blocking until task is finished
        }
    }

    auto prog = ui->FinishSort(options.output.value());
    RootWriter::Write(sorters.GetHistogram(), options.output.value());
    prog.Finish();
}

void TriggerSort(ProgressUI *ui, const CLI::Options &options)
{
    Task::Reader reader(options.input.value(), ui);
    Task::Converter converter(reader.GetQueue(), OptToTask(options.VetoAction.value()));
    Task::Buffer buffer(converter.GetQueue());
    Task::Splitter splitter(buffer.GetQueue(), options.coincidenceTime.value());

    Task::Triggers triggers(splitter.GetQueue(), options.coincidenceTime.value(), options.Trigger.value());
    Task::Sorters sorters(triggers.GetQueue(), options);

    // Spin up the worker threads
    std::vector<std::pair<std::thread, Task::Base *>> threads;
    threads.push_back(reader.ConstructThread());
    threads.push_back(converter.ConstructThread());
    threads.push_back(buffer.ConstructThread());
    threads.push_back(splitter.ConstructThread());
    for ( auto &trigger : triggers ){ threads.push_back(trigger->ConstructThread()); }
    for ( auto &sort : sorters ){ threads.push_back(sort->ConstructThread()); }

    // Now we just wait for everything to finish running
    for ( auto &runner : threads ){
        runner.second->Finish();
        if ( runner.first.joinable() ){
            runner.first.join(); // Blocking until task is finished
        }
    }

    auto prog = ui->FinishSort(options.output.value());
    RootWriter::Write(sorters.GetHistogram(), options.output.value());
    prog.Finish();
}

std::vector<std::string> GapSort(ProgressUI *ui, const CLI::Options &options)
{
    //std::vector<std::string> outname = OutNames(RemovePost(options.output.value()), 4);
    Task::Reader reader(options.input.value(), ui);
    Task::Converter converter(reader.GetQueue(), OptToTask(options.VetoAction.value()));
    Task::Buffer buffer(converter.GetQueue());
    Task::Splitter splitter(buffer.GetQueue(), options.coincidenceTime.value());
    Task::Trigger trigger(splitter.GetQueue(), -1, DetectorType::any);
    Task::Sorters sorters(trigger.GetQueue(), options, 8);

    // Spin up the worker threads
    std::vector<std::pair<std::thread, Task::Base *>> threads;
    threads.push_back(reader.ConstructThread());
    threads.push_back(converter.ConstructThread());
    threads.push_back(buffer.ConstructThread());
    threads.push_back(splitter.ConstructThread());
    threads.push_back(trigger.ConstructThread());
    for ( auto &sort : sorters ){ threads.push_back(sort->ConstructThread()); }

    // Now we just wait for everything to finish running
    for ( auto &runner : threads ){
        runner.second->Finish();
        if ( runner.first.joinable() ){
            runner.first.join(); // Blocking until task is finished
        }
    }
    RootWriter::Write(sorters.GetHistogram(), options.output.value());
    return {};
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
    std::vector<std::string> outfiles;
    switch ( options.sortType.value() ) {
        case CLI::sort_type::coincidence :
            TriggerSort_stt(&progress, options);
            break;
        case CLI::sort_type::gap :
            outfiles = GapSort(&progress, options);
            break;
    }

    // Merge files
    //auto prog = progress.FinishSort(options.output.value());
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