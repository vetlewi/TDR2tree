#include "CommandLineInterface.h"

#include "Calibration.h"

#include <structopt/app.hpp>
#include <structopt/third_party/magic_enum/magic_enum.hpp>

using namespace CLI;

STRUCTOPT(Options, input, output, CalibrationFile, coincidenceTime, tree, sortType, Trigger, addback, VetoAction);

size_t extract_file_no(const std::string &str)
{
    auto fno_start = str.find_last_of('_');
    if ( fno_start == std::string::npos ){
        throw std::runtime_error("Could not find file number");
    } else {
        fno_start += 1;
    }
    return std::stoi(std::string(str.begin()+fno_start, str.end()));
}

size_t get_file_no(const std::string &str)
{
    auto fname_start = str.find_last_of('/');
    if ( fname_start == std::string::npos ){
        fname_start = 0;
    } else {
        fname_start += 1;
    }
    try {
        return extract_file_no(std::string(str.begin() + fname_start, str.end()));
    } catch ( std::exception &ex ){
        throw std::runtime_error("Error parsing file path '" + str + "', got error: " + ex.what());
    }
}

bool compare_run_files(const std::string &lhs, const std::string &rhs)
{
    return get_file_no(lhs) < get_file_no(rhs);
}

void sort_file_names(std::vector<std::string> &files){
    std::sort(files.begin(), files.end(), compare_run_files);
}

std::ostream &operator<<(std::ostream &os, const Options &opt)
{
    os << "Sorting with following options:\n";
    os << "\tInput file(s):\n";
    for ( auto &file : opt.input.value() ){
        os << "\t\t" << file << "\n";
    }
    os << "\tOutput file: " << opt.output.value() << "\n";
    os << "\tCalibration file: " << opt.CalibrationFile.value_or("") << "\n";
    os << "\tCoincidence time: " << opt.coincidenceTime.value() << " ns\n";

    os << "\tBuild tree: " << std::boolalpha << opt.tree.value() << "\n";
    os << "\tSort type: " << magic_enum::enum_name(opt.sortType.value()) << "\n";
    os << "\tTrigger: " << magic_enum::enum_name(opt.Trigger.value()) << "\n";
    os << "\tAddback: " << std::boolalpha << opt.addback.value() << "\n";
    os << "\tBGO veto action: " << magic_enum::enum_name(opt.VetoAction.value()) << "\n";
    return os;
}

Options CLI::ParseCLA(const int &argc, char *argv[])
{
    Options options;
    try {
        structopt::app app("TDR2tree", "0.9.0");
        structopt::details::visitor vis("TDR2tree", "0.9.0");
        visit_struct::for_each(options, vis);
        options = app.parse<Options>(argc, argv);
        if ( !options.input.has_value() ){
            throw structopt::exception("Input(s) missing", vis);
        }
        if ( !options.output.has_value() ){
            throw structopt::exception("Output missing", vis);
        }
    } catch ( const structopt::exception &e ){
        std::cerr << e.what() << "\n";
        std::cout << e.help();
        throw e;
    }

    try {
        sort_file_names(options.input.value());
    } catch ( const std::exception &e ){
        std::cerr << e.what() << std::endl;
        throw e;
    }

    std::cout << options << std::endl;

    if ( options.CalibrationFile.has_value() ){
        if ( !SetCalibration(options.CalibrationFile.value().c_str()) ){
            std::cerr << "Error reading calibration file." << std::endl;
            throw std::runtime_error("Error reading calibration file");
        }
    }

    return options;
}