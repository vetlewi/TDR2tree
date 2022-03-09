#include "CommandLineInterface.h"

#include "Calibration.h"

#include <structopt/app.hpp>
#include <structopt/third_party/magic_enum/magic_enum.hpp>


using namespace CLI;

STRUCTOPT(Options, input, output, CalibrationFile, coincidenceTime, SplitTime, tree, sortType, Trigger, addback, VetoAction);

class parse_exception : public std::runtime_error
{
public:
    enum what_error {
        run,
        filenumber,
        both
    };

    explicit parse_exception(const what_error &_error_type, const std::string &filename = "")
        : std::runtime_error("Could not parse file")
        , error_type( _error_type )
        , fname( filename )
    {}

    virtual ~parse_exception() throw () {}

    [[nodiscard]] what_error GetError() const { return error_type; }
    [[nodiscard]] std::string Filename() const { return fname; }

private:
    what_error error_type;
    std::string fname;
};

std::ostream &operator<<(std::ostream &os, const parse_exception &ex)
{
    switch ( ex.GetError() ) {
        case parse_exception::run :
            os << "Unable to find run number in file '" << ex.Filename() << "'";
            break;
        case parse_exception::filenumber :
            os << "Unable to find file number in file '" << ex.Filename() << "'";
            break;
        case parse_exception::both :
            os << "Unable to parse files";
            break;
        default :
            throw std::runtime_error("Unexpected error in parse_exception");
    }
    return os;
}

size_t extract_run_no(const std::string &str)
{
    auto fno_start = str.find_last_of('R');
    auto fno_end = str.find_last_of('_');
    if ( fno_start == std::string::npos || fno_end == std::string::npos ){
        throw parse_exception(parse_exception::run, str);
    } else {
        fno_start += 1;
        fno_end += 1;
    }
    try {
        return std::stoi(std::string(str.begin() + fno_start, str.begin() + fno_end));
    } catch ( const std::invalid_argument &ex ){
        throw parse_exception(parse_exception::run, str);
    }
}

size_t extract_file_no(const std::string &str)
{
    auto fno_start = str.find_last_of('_');
    if ( fno_start == std::string::npos ){
        throw parse_exception(parse_exception::filenumber, str);
    } else {
        fno_start += 1;
    }
    try {
        return std::stoi(std::string(str.begin() + fno_start, str.end()));
    } catch ( const std::invalid_argument &ex ) {
        throw parse_exception(parse_exception::filenumber, str);
    }
}

size_t get_run_no(const std::string &str)
{
    auto fname_start = str.find_last_of('/');
    if ( fname_start == std::string::npos ){
        fname_start = 0;
    } else {
        fname_start += 1;
    }
    return extract_run_no(std::string(str.begin()+fname_start, str.end()));
}

size_t get_file_no(const std::string &str)
{
    auto fname_start = str.find_last_of('/');
    if ( fname_start == std::string::npos ){
        fname_start = 0;
    } else {
        fname_start += 1;
    }
    return extract_file_no(std::string(str.begin() + fname_start, str.end()));
}

bool compare_run_files(const std::string &lhs, const std::string &rhs)
{
    try {
        return (get_run_no(lhs) == get_run_no(rhs)) ? (get_file_no(lhs) < get_file_no(rhs)) : (get_run_no(lhs) < get_run_no(rhs));
    } catch ( const parse_exception &ex ){
        std::cerr << ex << std::endl;
        if ( ex.GetError() == parse_exception::run ){
            try {
                return (get_file_no(lhs) < get_file_no(rhs));
            } catch ( const parse_exception &ex2 ){
                std::cerr << ex2 << std::endl;
                throw parse_exception(parse_exception::both);
            }
        } else {
            try {
                return (get_run_no(lhs) < get_run_no(rhs));
            } catch ( const parse_exception &ex2 ){
                std::cerr << ex2 << std::endl;
                throw parse_exception(parse_exception::both);
            }
        }
    }
}

void sort_file_names(std::vector<std::string> &files){
    try {
        std::sort(files.begin(), files.end(), compare_run_files);
    } catch ( const parse_exception &ex ){
        std::cerr << "Unable to parse filename. Will not be sorted." << std::endl;
    }
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
    os << "\tSplit time: " << opt.SplitTime.value() << " ns\n";

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