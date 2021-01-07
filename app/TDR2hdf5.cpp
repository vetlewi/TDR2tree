//
// Created by Vetle Wegner Ingeberg on 01/10/2020.
//

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <exception>

#include <spdlog/spdlog.h>
#include <CLI/CLI.hpp>

#include <Buffer/Buffer.h>
#include <Buffer/BufferFetcher.h>
#include <Buffer/MTFileBufferFetcher.h>
#include <Parser/TDRparser.h>
#include <Utilities/HDF5_writer.h>
#include <Utilities/ProgressUI.h>

ProgressUI progress;

struct RunSettings {
    spdlog::level::level_enum log_level{spdlog::level::level_enum::info};

    std::vector<std::string> input_files;
    std::string output_file;
    std::string calibration_file;

    size_t threads{1};
    bool addback{false};
};

size_t CountTDREvents(const std::vector<std::string> &files)
{
    auto buffer_type = Fetcher::TDRBuffer();
    auto *bf = new Fetcher::MTFileBufferFetcher(&buffer_type);
    auto parser = Parser::TDRparser();
    const Fetcher::Buffer *buf;
    std::vector<Parser::Entry_t> entries;
    size_t eventsFound = 0;


    for (auto &file : files){
        auto status = bf->Open(file.c_str(), 0);
        if (status != Fetcher::BufferFetcher::OKAY){
            throw std::runtime_error("Unable to read file '" + file + "'.");
        }

        while (true){
            buf = bf->Next(status);
            if ( status != Fetcher::BufferFetcher::OKAY ){
                break;
            }
            eventsFound += parser.GetEntry(buf).size();
        }
    }
    delete bf;
    return eventsFound;
}



void SetupCLI(CLI::App &app, RunSettings *settings)
{
    app.description("TDR2HDF5 - A tool to convert TDR files to HDF5 files.");

    app.add_option("-i,--input", settings->input_files,
                   "Input file(s).")->required(true);
    app.add_option("-o,--output", settings->output_file,
                   "Output file.")->required(true);

    std::string lvl;
    app.add_option("-l,--level", lvl,
                   "Set logging level.")
                   ->default_str("info");
    settings->log_level = spdlog::level::from_str(lvl);

    app.add_option("-t,--threads",
                   settings->threads)->default_val(std::thread::hardware_concurrency());

    app.add_flag("-a,--addback", settings->addback,
                 "Turn addback on or off")->default_val(true);
}


int main(int argc, char* argv[])
{
    // Read command line settings
    CLI::App app;
    RunSettings settings;
    SetupCLI(app, &settings);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    // Next we will need to get the total number of events in the input file.
    size_t nEvents = CountTDREvents(settings.input_files);

    // Now we setup the file writer
    HDF5_Writer writer;

    try {
        writer.allocate(settings.output_file.c_str(), nEvents);
    } catch (std::exception &e) {
        spdlog::error("Creating output file failed. Error %s", e.what());
    }

    // Read and write to file
    auto buffer_type = Fetcher::TDRBuffer();
    auto *bf = new Fetcher::MTFileBufferFetcher(&buffer_type);
    auto parser = Parser::TDRparser();
    const Fetcher::Buffer *buf;
    std::vector<Parser::Entry_t> entries;
    for (auto &file : settings.input_files){
        auto status = bf->Open(file.c_str(), 0);
        if (status != Fetcher::BufferFetcher::OKAY){
            throw std::runtime_error("Unable to read file '" + file + "'.");
        }

        while (true){
            buf = bf->Next(status);
            if ( status != Fetcher::BufferFetcher::OKAY ){
                break;
            }
            entries = parser.GetEntry(buf);
            for ( auto &entry : entries ){
                if (!writer.Write(entry))
                    break;
            }
        }
    }
    delete buf;

    return 0;
}