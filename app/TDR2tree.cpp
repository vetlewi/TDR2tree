//
// Created by Vetle Wegner Ingeberg on 21/10/2019.
//

// C++ STD libs
#include <iostream>

// C libs
#include <cstdio>

// External dependencies
#include <spdlog/spdlog.h>
#include <spdlog/sinks/file_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>

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

#include <TROOT.h>

ProgressUI progress; // NOLINT(cert-err58-cpp)

#include <Utilities/CLI_interface.h>
#include <CLI11.hpp>

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
}

int main(int argc, char* argv[])
{
    //ROOT::EnableThreadSafety();
    auto file_logger = spdlog::basic_logger_mt("logger", "log.txt");
    auto console = spdlog::stdout_color_mt("console");
    file_logger->set_level(spdlog::level::info);

    Settings_t settings = {
            std::vector<std::string>(),
            "",
            "TDR2tree",
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
            1,
            1
    };

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
    app.add_option("--TreeName", settings.tree_name, "Name of the tree. Default is 'events'")->default_str("events");
    app.add_option("--TreeTitle", settings.tree_title, "Title of the tree. Default is 'Events'")->default_str("Events");
    app.add_option("-f,--format", format, "Input file format. Default TDR.")
        ->default_str("TDR")
        ;//->transform(CLI::CheckedTransformer(format_map, CLI::ignore_case));
    app.add_option("--trigger", settings.trigger_type, "Detector event trigger. Default is eDet")
        ->default_str("eDet")
        ;//->transform(CLI::CheckedTransformer(trigger_map, CLI::ignore_case));
    app.add_option("--queue_size", Queue_size, "Maximum size of the internal queues. Default is 8192")
        ->default_val("8192");
    app.add_option("--SplitThreads", settings.num_split_threads, "Number of splitter threads. Default is 1")
        ->default_val("1");
    app.add_option("--FillThreads", settings.num_filler_threads,
            "Number of filler threads. Default is 1. Note that ROOT often causes errors when multiple threads tries to interact with ROOT")
        ->default_val("1");

    try {
        app.parse(argc, argv);
    } catch ( const CLI::ParseError &e ){
        return app.exit(e);
    }

    SetCalibration(calfile.c_str());

    std::cout << "Output file: " << settings.output_file << std::endl;

    // First we need to check if the format is implemented.
    switch ( format ){

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
        case Sirius : {
            std::cout << "Sirius format is not yet implemented." << std::endl;
            return 0;
        }
#pragma clang diagnostic pop

        case TDR : {
            SetupTDR(settings, Queue_size);
            break;
        }
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
        case XIA : {
            std::cout << "XIA format is not yet implemented." << std::endl;
            return 0;
        }
#pragma clang diagnostic pop
    }

#if ROOT_MT_FLAG
    ROOT::EnableImplicitMT(settings.num_filler_threads);
#else
    std::cout << "INFO: FillThread flag ignored, single thread filler used." << std::endl;
#endif // ROOT_MT_FLAG

    // Next we will start the converter.
    ConvertFiles(&settings);
    return 0;

}
