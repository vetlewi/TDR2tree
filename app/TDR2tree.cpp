//
// Created by Vetle Wegner Ingeberg on 21/10/2019.
//

// C++ STD libs
#include <iostream>

// C libs
#include <cstdio>

// ROOT stuff first?!
#if ROOT_MT_FLAG
#include <TROOT.h>
#endif // ROOT_MT_FLAG

// External dependencies
#if LOG_ENABLED
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#endif // LOG_ENABLED

// Buffer library
#include <Buffer/Buffer.h>

// Parser library
#include <Parser/TDRparser.h>

// Event library
#include <Event/iThembaEvent.h>
#include <Parameters/experimentsetup.h>
#include <Parameters/Calibration.h>

// Utillities library
#include <Utilities/ProgressUI.h>

ProgressUI progress; // NOLINT(cert-err58-cpp)

#include <Utilities/CLI_interface.h>
#include <CLI/CLI.hpp>

#include "SortUtillities.h"

enum Format {
    Sirius,
    TDR,
    XIA
};

void SetupTDR(Settings_t &settings, const int &queue_size)
{
    settings.buffer_type = new Fetcher::TDRBuffer;
    settings.parser = new Parser::TDRparser;
    settings.event_type = new Event::iThembaEvent;
    settings.input_queue = new Entry_queue_t(queue_size);
    settings.split_queue = new Event_queue_t(queue_size);
    settings.built_queue = new Event_queue_t(queue_size);
    settings.str_queue = new String_queue_t(queue_size);

    if ( settings.build_tree && settings.output_csv ){
        std::cerr << "Error: Cannot output CSV and tree at the same time." << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    //ROOT::EnableThreadSafety();
#if LOG_ENABLED
    auto file_logger = spdlog::basic_logger_mt("logger", "log.txt");
    auto console = spdlog::stdout_color_mt("console");
    file_logger->set_level(spdlog::level::info);
#endif // LOG_ENABLED

    Settings_t settings = {
            std::vector<std::string>(),
            "",
            "TDR2tree",
            false,
            false,
            "events",
            "Events",
            nullptr,
            nullptr,
            nullptr,
            1500,
            1500,
            DetectorType::eDet,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            2,
            1,
            false
    };

    std::string config_out = "";

    CLI::App app{"TDR2tree - a list-mode converter and event builder"};

    std::string calfile;
    Format format = Format::TDR;
    std::vector<std::pair<std::string, Format> > format_map{
        {"Sirius", Format::Sirius}, {"TDR", Format::TDR}, {"XIA", Format::XIA}};
    std::vector<std::pair<std::string, DetectorType> > trigger_map{
        {"invalid", DetectorType::invalid},
        {"labr_3x8", DetectorType::labr_3x8},
        {"labr_2x2_ss", DetectorType::labr_2x2_ss},
        {"labr_2x2_fs", DetectorType::labr_2x2_fs},
        {"clover", DetectorType::clover},
        {"de_ring", DetectorType::de_ring},
        {"de_sect", DetectorType::de_sect},
        {"eDet", DetectorType::eDet},
        {"rfchan", DetectorType::rfchan},
        {"any", DetectorType::any},
        {"unused", DetectorType::unused}
    };

    size_t Queue_size = 0x2000;

    app.add_option("-i,--input", settings.input_files, "Input file(s)")->required();
    app.add_option("-o,--output", settings.output_file, "Output file")->required();
    app.add_option("-c,--calibration", calfile, "Calibration file");
    app.add_option("-s,--SplitTime", settings.split_time,
            "Time gap between entries where data are split. Default is 1500 ns")->default_val("1500");
    app.add_option("-e,--EventTime", settings.event_time,
            "Maximum time difference for an entry to be included in an event. Default is 1500 ns")->default_val("1500");
    app.add_flag("-t,--tree", settings.build_tree, "Flag to indicate that a tree should be built");
    app.add_flag("--csv", settings.output_csv, "Flag to indicate that output should be compressed CSV (zlib). Cannot be selected together with -t,--tree");
    app.add_option("--TreeName", settings.tree_name, "Name of the tree. Default is 'events'")->default_str("events");
    app.add_option("--TreeTitle", settings.tree_title, "Title of the tree. Default is 'Events'")->default_str("Events");
    app.add_option("-f,--format", format, "Input file format. Default TDR.")
        ->default_str("TDR")->transform(CLI::CheckedTransformer(format_map, CLI::ignore_case));
    app.add_option("--trigger", settings.trigger_type, "Detector event trigger. Default is eDet")
        ->default_str("eDet")->transform(CLI::CheckedTransformer(trigger_map, CLI::ignore_case));
    app.add_option("--queue_size", Queue_size, "Maximum size of the internal queues. Default is 8192")
        ->default_val("8192");
    app.add_option("--SplitThreads", settings.num_split_threads, "Number of splitter threads. Default is 1")
        ->default_val("1");
    app.add_option("--FillThreads", settings.num_filler_threads,
            "Number of filler threads. Default is 1. Note that ROOT often causes errors when multiple threads tries to interact with ROOT")
        ->default_val("1");
    app.add_flag("--PR271", settings.PR271, "Exp. PR271 - removes the pulser");
    app.add_option("--write-config", config_out, "File to write config to.");
    app.set_config("--config");
    app.config_formatter(std::make_shared<CLI::ConfigTOML>());
    try {
        app.parse(argc, argv);
    } catch ( const CLI::ParseError &e ){
        return app.exit(e);
    }
    auto input_files = settings.input_files;
    settings.input_files.clear();
    for ( auto &input : input_files ){
        if ( !input.empty() )
            settings.input_files.push_back(input);
    }

    SetCalibration(calfile.c_str());

    if ( !config_out.empty() ){
        std::ofstream outfile(config_out);
        outfile << app.config_to_str(true, true);
    }

    std::cout << "Calibration file: " << calfile << std::endl;
    auto trig = std::find_if(std::begin(trigger_map), std::end(trigger_map),
            [&settings](const std::pair<std::string, DetectorType> &i){
        return i.second == settings.trigger_type; });
    std::cout << "Trigger: " << trig->first << std::endl;

    std::cout << "Splitter threads: " << settings.num_split_threads << std::endl;
    std::cout << "Filler threads: " << settings.num_filler_threads << std::endl;
    std::cout << "Input format: ";
    // First we need to check if the format is implemented.
    switch ( format ){

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
        case Sirius : {
            //std::cout << "Sirius" << std::endl;
            std::cout << "Sirius format is not yet implemented." << std::endl;
            return 0;
        }
#pragma clang diagnostic pop

        case TDR : {
            std::cout << "TDR" << std::endl;
            SetupTDR(settings, Queue_size);
            break;
        }
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
        case XIA : {
            //std::cout << "XIA" << std::endl;
            std::cout << "XIA format is not yet implemented." << std::endl;
            return 0;
        }
#pragma clang diagnostic pop
    }

    std::cout << "Ouput format: ";
    if ( settings.output_csv )
        std::cout << " CSV" << std::endl;
    else
        std::cout << " ROOT" << std::endl;
    std::cout << "Output file: " << settings.output_file << std::endl;

#if ROOT_MT_FLAG
    ROOT::EnableImplicitMT(settings.num_filler_threads);
#else
    std::cout << "INFO: FillThread flag ignored, single thread filler used." << std::endl;
#endif // ROOT_MT_FLAG

    // Next we will start the converter.
#if POSTGRESQL_ENABLED
    ConvertPostgre(&settings);
#else
    if ( settings.output_csv )
        ConvertFilesCSV(&settings);
    else
        ConvertFiles(&settings);

#endif // POSTGRESQL_ENABLED
    return 0;

}
