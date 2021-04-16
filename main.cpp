#include <iostream>

#include <string>
#include <deque>
#include <vector>
#include <algorithm>

#include "CommandLineInterface.h"
#include "Calibration.h"
#include "experimentsetup.h"

#include "RootFileManager.h"
#include "HistManager.h"
#include "TreeManager.h"
#include "Event.h"
#include "ProgressUI.h"
#include "BufferType.h"
#include "Histograms.h"
#include "TDREntry.h"
#include "TDRParser.h"
#include "TDRTypes.h"
#include "XIA_CFD.h"
#include "MemoryMap.h"
#include "TreeEvent.h"

#include <structopt/app.hpp>
#include <structopt/third_party/magic_enum/magic_enum.hpp>



ProgressUI progress;

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

enum veto_action {
    nothing,
    remove,
    keep
};

enum sort_type {
    coincidence,
    gap,
    singles
};

struct Options
{
    // Required arguments
    std::optional<std::vector<std::string>> input;
    std::optional<std::string> output;

    // Optional arguments
    std::optional<std::string> CalibrationFile;
    std::optional<int> coincidenceTime = 1500;
    std::optional<bool> tree = false;
    std::optional<sort_type> sortType = sort_type::singles;
    std::optional<DetectorType> Trigger = DetectorType::eDet;
    std::optional<bool> addback = false;
    std::optional<veto_action> VetoAction = veto_action::nothing;
    std::optional<bool> batch_read = false;
};

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
    os << "\tBatch read: " << std::boolalpha << opt.batch_read.value();
    return os;
}

STRUCTOPT(Options, input, output, CalibrationFile, coincidenceTime, tree, sortType, Trigger, addback, VetoAction, batch_read);

inline int64_t TSFactor(const ADCSamplingFreq &freq)
{
    if ( freq == f250MHz )
        return 8;
    else
        return 10;
}

std::vector<word_t> TDRtoWord(const std::vector<TDR::Entry_t> &entries, const enum veto_action &action = veto_action::remove)
{
    const DetectorInfo_t *dinfo;
    std::vector<word_t> words;
    words.reserve(entries.size());
    word_t word{};
    unsigned short adc_data;
    XIA::XIA_CFD_t cfd_res;

    for ( auto &entry : entries ){
        // This is also the place where we will remove any events with adc larger than 16384.
        adc_data = entry.adc->ADC_data;
        if ( action != veto_action::nothing && ( adc_data & 0x8000 ) == 0x8000 ) {
            if ( action == veto_action::remove ){
                continue;
            } else if ( action == veto_action::keep ){
                adc_data &= 0x7FFF;
            }
        }
        word = {entry.GetAddress(),
                adc_data,
                uint16_t(entry.tdc->ADC_data),
                entry.adc->fail,
                entry.adc->veto || (( adc_data & 0x8000 ) == 0x8000),
                entry.timestamp(),
                0,
                true,
                0};
        dinfo = GetDetectorPtr(word.address);
        cfd_res = XIA::XIA_CFD_Decode(dinfo->sfreq, word.cfddata);
        word.cfdcorr = cfd_res.first;
        word.cfdfail = cfd_res.second;

        word.energy = CalibrateEnergy(word);
        word.cfdcorr += CalibrateTime(word);

        word.timestamp *= TSFactor(dinfo->sfreq);
        word.timestamp += int64_t(word.cfdcorr);
        word.cfdcorr -= int64_t(word.cfdcorr);
        words.push_back(word);
    }
    return words;
}

std::vector<word_t> TDRtoWord_prog(const std::string &fname, const std::vector<TDR::Entry_t> &entries, const enum veto_action &action = veto_action::remove)
{
    const DetectorInfo_t *dinfo;
    std::vector<word_t> words;
    word_t word{};
    XIA::XIA_CFD_t cfd_res;
    unsigned short adc_data;
    progress.StartNewFile(fname, entries.size());
    size_t pos = 0;
    for ( auto &entry : entries ){
        // This is also the place where we will remove any events with adc larger than 16384.
        adc_data = entry.adc->ADC_data;
        if ( action != veto_action::nothing && ( adc_data & 0x8000 ) == 0x8000 ) {
            if ( action == veto_action::remove ){
                ++pos;
                continue;
            } else if ( action == veto_action::keep ){
                adc_data &= 0x7FFF;
            }
        }
        word = {entry.GetAddress(),
                adc_data,
                uint16_t(entry.tdc->ADC_data),
                entry.adc->fail,
                entry.adc->veto || (( adc_data & 0x8000 ) == 0x8000),
                entry.timestamp(),
                0,
                true,
                0};
        dinfo = GetDetectorPtr(word.address);
        cfd_res = XIA::XIA_CFD_Decode(dinfo->sfreq, word.cfddata);
        word.cfdcorr = cfd_res.first;
        word.cfdfail = cfd_res.second;

        word.energy = CalibrateEnergy(word);
        word.cfdcorr += CalibrateTime(word);

        word.timestamp *= TSFactor(dinfo->sfreq);
        word.timestamp += int64_t(word.cfdcorr);
        word.cfdcorr -= int64_t(word.cfdcorr);
        words.push_back(word);

        progress.UpdateReadProgress(++pos);
    }
    return words;
}

void Convert_to_root_MM_all(const std::vector<std::string> &in_files, const std::string &out_file, const Options &opt)
{
    RootFileManager fm(out_file.c_str());
    HistManager hm(&fm);
    TreeManager<TreeEvent> tm(&fm, "events", "Event tree");
    TreeManager<TreeEvent> *tm_ptr = ( opt.tree.value() ) ? &tm : nullptr;
    Histogram2Dp ab_hist = ( opt.addback.value() ) ? fm.Mat("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr;

    std::function<void(std::vector<word_t>::iterator, std::vector<word_t>::iterator)> sort_func;
    switch ( opt.sortType.value() ) {
        case sort_type::coincidence : {
            sort_func = [&hm, &tm_ptr, &ab_hist, &opt](std::vector<word_t>::iterator start,
                                                       std::vector<word_t>::iterator stop){
                Event::BuildPGAndFill(start, stop, &hm, tm_ptr, ab_hist, opt.Trigger.value(), opt.coincidenceTime.value(), &progress);
            };
            break;
        }

        case sort_type::gap : {
            sort_func = [&hm, &tm_ptr, &ab_hist, &opt](std::vector<word_t>::iterator start,
                                                       std::vector<word_t>::iterator stop){
                Event::BuildAndFill(start, stop, &hm, tm_ptr, ab_hist, opt.coincidenceTime.value(), &progress);
            };
        }

        case sort_type::singles : {
            sort_func = [&hm, &tm_ptr, &ab_hist, &opt](std::vector<word_t>::iterator start,
                                                       std::vector<word_t>::iterator stop){
                hm.AddEntries(start, stop);
            };
            break;
        }
    }

    for ( auto &file : in_files ) {
        IO::MemoryMap mmap(file.c_str());
        progress.StartNewFile(file, mmap.GetSize());
        auto events = TDRtoWord_prog(file, TDR::ParseFile(mmap.GetPtr(), mmap.GetPtr() + mmap.GetSize()),
                                     opt.VetoAction.value());
        sort_func(events.begin(), events.end());
        progress.Finish();
    }
}

void Convert_to_root_MM(const std::vector<std::string> &in_files, const std::string &out_file, const Options &opt)
{
    std::vector<Event> event_data;
    RootFileManager fm(out_file.c_str());
    HistManager hm(&fm);
    TreeManager<TreeEvent> tm(&fm, "events", "Event tree");
    TreeManager<TreeEvent> *tm_ptr = ( opt.tree.value() ) ? &tm : nullptr;
    Histogram2Dp ab_hist = ( opt.addback.value() ) ? fm.Mat("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr;

    TDR::Parser parser;
    std::vector<word_t> data;
    std::vector<word_t> buffer;


    std::vector<std::unique_ptr<IO::MemoryMap>> mem_maps;
    for ( auto &file : in_files ){
        mem_maps.emplace_back(new IO::MemoryMap(file.c_str()));
    }

    std::function<void(std::vector<word_t>::iterator, std::vector<word_t>::iterator)> sort_func;
    switch ( opt.sortType.value() ) {
        case sort_type::coincidence : {
            sort_func = [&hm, &tm_ptr, &ab_hist, &opt](std::vector<word_t>::iterator start,
                                                       std::vector<word_t>::iterator stop){
                Event::BuildPGAndFill(start, stop, &hm, tm_ptr, ab_hist, opt.Trigger.value(), opt.coincidenceTime.value());
            };
            break;
        }
        case sort_type::gap : {
            sort_func = [&hm, &tm_ptr, &ab_hist, &opt](std::vector<word_t>::iterator start,
                                                       std::vector<word_t>::iterator stop){
                Event::BuildAndFill(start, stop, &hm, tm_ptr, ab_hist, opt.coincidenceTime.value());
            };
            break;
        }
        case sort_type::singles : {
            sort_func = [&hm, &tm_ptr, &ab_hist, &opt](std::vector<word_t>::iterator start,
                                                       std::vector<word_t>::iterator stop){
                hm.AddEntries(start, stop);
            };
            break;
        }
    }

    size_t fno = 0;
    for ( auto &mmap : mem_maps ){
        progress.StartNewFile(in_files[fno++], mmap->GetSize());
        bool end = (fno == mem_maps.size());
        const char *header = TDR::FindHeader(mmap->GetPtr(), mmap->GetPtr() + mmap->GetSize());
        const char *stop = mmap->GetPtr() + mmap->GetSize();
        while ( header < stop  ){
            const char *next_header = TDR::FindHeader(header + reinterpret_cast<const TDR::TDR_header_t *>(header)->header_dataLen + sizeof(TDR::TDR_header_t), stop);
            buffer = TDRtoWord(parser.ParseBuffer(header, (next_header >= stop) && end ), opt.VetoAction.value());
            data.insert(data.end(), buffer.begin(), buffer.end());
            std::sort(data.begin(), data.end(), [](const word_t &lhs, const word_t &rhs)
            { return (double(rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });
            if ( data.size() > 196608 ){
                sort_func(data.begin(), data.begin()+65536);
                data.erase(data.begin(), data.begin()+65536);
            }
            progress.UpdateReadProgress(header - mmap->GetPtr());
            header = next_header;
        }
        progress.Finish();
    }

    // Once we reach this point we will flush the remaining events.
    sort_func(data.begin(), data.end());
}



int main(int argc, char *argv[])
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
        exit(EXIT_FAILURE);
    }

    try {
        sort_file_names(options.input.value());
    } catch ( const std::exception &e ){
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }

    // Check that we don't try to sort singles and build tree at the same time
    if ( options.sortType.value() == sort_type::singles && options.tree.value() ){
        std::cerr << "Error: Cannot sort singles into trees." << std::endl;
        exit(EXIT_FAILURE);
    }

    // Next we will now print all arguments:
    std::cout << options << std::endl;

    if ( options.CalibrationFile.has_value() ){
        if ( !SetCalibration(options.CalibrationFile.value().c_str()) ){
            std::cerr << "Error reading calibration file." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    if ( options.batch_read.value() )
        Convert_to_root_MM_all(options.input.value(), options.output.value(), options);
    else
        Convert_to_root_MM(options.input.value(), options.output.value(), options);
    exit(EXIT_SUCCESS);
}

