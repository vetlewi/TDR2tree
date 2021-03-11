#include <iostream>

#include <string>
#include <deque>
#include <vector>
#include <algorithm>
#include <queue>

#include "CommandLineInterface.h"
#include "Calibration.h"
#include "experimentsetup.h"

#include "RootFileManager.h"
#include "HistManager.h"
#include "TreeManager.h"
#include "Event.h"
#include "ProgressUI.h"
#include "MTFileBufferFetcher.h"
#include "BufferType.h"

#include <TROOT.h>

#include <TDREntry.h>
#include <TDRParser.h>
#include <TDRTypes.h>
#include <XIA_CFD.h>
#include <MemoryMap.h>

ProgressUI progress;
//PolygonGate sectBackGate;
//PolygonGate ringSectGate;

struct Options
{
    int coincidence_time;
    bool build_tree;
    bool particle_gamma;
    bool addback;
    bool validate;
    bool remove_overflow;
};

inline int64_t TSFactor(const ADCSamplingFreq &freq)
{
    if ( freq == f100MHz || freq == f500MHz )
        return 10;
    else if ( freq == f250MHz )
        return 8;
    else
        return 10;
}

std::vector<word_t> TDRtoWord(const std::vector<TDR::Entry_t> &entries, const bool &remove_of = true)
{
    DetectorInfo_t dinfo;
    std::vector<word_t> words;
    words.reserve(entries.size());
    word_t word;
    XIA::XIA_CFD_t cfd_res;
    for ( auto &entry : entries ){
        // This is also the place where we will remove any events with adc larger than 16384.
        if ( (entry.adc->ADC_data >= 16384 && remove_of) || (entry.GetAddress() == 486) )
            continue;
        word = {entry.GetAddress(),
                uint16_t(entry.adc->ADC_data),
                uint16_t(entry.tdc->ADC_data),
                entry.adc->fail,
                entry.adc->veto,
                entry.timestamp(),
                true,
                0};
        dinfo = GetDetector(word.address);
        cfd_res = XIA::XIA_CFD_Decode(dinfo.sfreq, word.cfddata);
        word.cfdcorr = cfd_res.first;
        word.cfdfail = cfd_res.second;

        word.cfdcorr += CalibrateTime(word);

        word.timestamp *= TSFactor(dinfo.sfreq);
        word.timestamp += int64_t(word.cfdcorr);
        word.cfdcorr -= int64_t(word.cfdcorr);
        words.push_back(word);
    }
    return words;
}

std::vector<word_t> TDRtoWord_prog(const std::string &fname, const std::vector<TDR::Entry_t> &entries, const bool &remove_of = true)
{
    DetectorInfo_t dinfo;
    std::vector<word_t> words;
    word_t word;
    XIA::XIA_CFD_t cfd_res;
    progress.StartNewFile(fname, entries.size());
    size_t pos = 0;
    for ( auto &entry : entries ){
        if ( (entry.adc->ADC_data >= 16384 && remove_of) || (entry.GetAddress() == 486) ) {
            ++pos;
            continue;
        }
        word = {entry.GetAddress(),
                uint16_t(entry.adc->ADC_data),
                uint16_t(entry.tdc->ADC_data),
                entry.adc->fail,
                entry.adc->veto,
                entry.timestamp(),
                true,
                0};
        dinfo = GetDetector(word.address);
        try {
            cfd_res = XIA::XIA_CFD_Decode(dinfo.sfreq, word.cfddata);
        } catch (...){
            continue; // skip if fail
        }
        word.cfdcorr = cfd_res.first;
        word.cfdfail = cfd_res.second;

        word.cfdcorr += CalibrateTime(word);

        word.timestamp *= TSFactor(dinfo.sfreq);
        word.timestamp += int64_t(word.cfdcorr);
        word.cfdcorr -= int64_t(word.cfdcorr);
        words.push_back(word);

        progress.UpdateReadProgress(++pos);
    }
    return words;
}

std::ostream &operator<<(std::ostream &os, const Options &opt)
{
    os << "\tCoincidence time: " << opt.coincidence_time << " ns\n";
    os << "\tAddback: " << ( opt.addback ? "true" : "false") << "\n";
    os << "\tParticle gamma: " << ( opt.particle_gamma ? "true" : "false") << "\n";
    os << "\tBuild tree: " << ( opt.build_tree ? "true" : "false") << "\n";
    os << "\tValidate: " << ( opt.validate ? "true " : "false") << "\n";
    os << "\tRemove overflow: " << std::boolalpha << opt.remove_overflow;
    return os;
}

void Convert_to_root(const std::vector<std::string> &in_files, const std::string &out_file, const Options &opt)
{
    std::vector<Event> event_data;
    RootFileManager fm(out_file.c_str());
    HistManager hm(&fm);
    TreeManager<Event> tm(&fm, "events", "Event tree", opt.validate);
    TH2 *ab_hist = ( opt.addback ) ? fm.CreateTH2("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr;
    for ( const auto& file : in_files ) {
        //event_data = ( pg_event ) ? Event::BuildPGEvents(FileReader::GetFile(file.c_str()), ab_hist) : Event::BuildEvent(FileReader::GetFile(file.c_str()), ab_hist);
        //hm.AddEntries(event_data);
        //tm.AddEntries(event_data);
        if ( opt.particle_gamma )
            Event::BuildPGAndFill(FileReader::GetFile(file.c_str()), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
        else
            Event::BuildAndFill(FileReader::GetFile(file.c_str()), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
        progress.Finish();
    }
}

void Convert_to_root_MT(const std::vector<std::string> &in_files, const std::string &out_file, const Options &opt)
{
    std::vector<Event> event_data;
    RootFileManager fm(out_file.c_str());
    HistManager hm(&fm);
    TreeManager<Event> tm(&fm, "events", "Event tree", opt.validate);
    TH2 *ab_hist = ( opt.addback ) ? fm.CreateTH2("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr;
    for ( const auto& file : in_files ) {
        MTFileBufferFetcher bufFetch;
        bufFetch.Open(file);
        BufferFetcher::Status status;
        std::vector<word_t> data;
        const TDRBuffer *buffer;
        while ( true ){
            buffer = bufFetch.Next(status);
            if ( status == BufferFetcher::OKAY ){
                data.insert(data.end(), buffer->CGetBuffer(), buffer->CGetBuffer()+buffer->GetSize());
                if ( data.size() > 196608 ){
                    std::sort(data.begin(), data.end(), [](const word_t &lhs, const word_t &rhs) { return ((rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });
                    if ( opt.particle_gamma )
                        Event::BuildPGAndFill(std::vector<word_t>(data.begin(), data.begin()+65536), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
                    else
                        Event::BuildAndFill(std::vector<word_t>(data.begin(), data.begin()+65536), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
                    std::vector<word_t> new_data(data.begin()+65536, data.end());
                    data = new_data;
                }
            } else if ( status == BufferFetcher::END ) {
                if ( buffer )
                    data.insert(data.end(), buffer->CGetBuffer(), buffer->CGetBuffer()+buffer->GetSize());
                std::sort(data.begin(), data.end(), [](const word_t &lhs, const word_t &rhs) { return ((rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });
                if ( opt.particle_gamma )
                    Event::BuildPGAndFill(std::vector<word_t>(data.begin(), data.end()), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
                else
                    Event::BuildAndFill(std::vector<word_t>(data.begin(), data.end()), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
                progress.Finish();
                break;
            } else {
                break;
            }
        }
    }
}

void Convert_to_root_MM_all(const std::vector<std::string> &in_files, const std::string &out_file, const Options &opt)
{
    RootFileManager fm(out_file.c_str());
    HistManager hm(&fm);
    TreeManager<Event> tm(&fm, "events", "Event tree", opt.validate);
    TH2 *ab_hist = ( opt.addback ) ? fm.CreateTH2("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr;

    for ( auto &file : in_files ) {
        IO::MemoryMap mmap(file.c_str());
        progress.StartNewFile(file, mmap.GetSize());
        auto events = TDRtoWord_prog(file, TDR::ParseFile(mmap.GetPtr(), mmap.GetPtr() + mmap.GetSize()),
                                     opt.remove_overflow);
        if ( opt.particle_gamma )
            Event::BuildPGAndFill(events, &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time, &progress);
        else
            Event::BuildAndFill(events, &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
        progress.Finish();
    }

}

void Convert_to_root_MM(const std::vector<std::string> &in_files, const std::string &out_file, const Options &opt)
{
    std::vector<Event> event_data;
    RootFileManager fm(out_file.c_str());
    HistManager hm(&fm);
    TreeManager<Event> tm(&fm, "events", "Event tree", opt.validate);
    TH2 *ab_hist = ( opt.addback ) ? fm.CreateTH2("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr;

    for ( auto &file : in_files ){
        IO::MemoryMap mmap(file.c_str());
        TDR::Parser parser;
        std::deque<word_t> data;
        std::vector<word_t> buffer;
        std::vector<const char *> headers = TDR::FindHeaders(mmap.GetPtr(), mmap.GetPtr() + mmap.GetSize());
        progress.StartNewFile(file, headers.size());
        for ( size_t hi = 0 ; hi < headers.size() ; ++hi ){
            buffer = TDRtoWord(parser.ParseBuffer(headers[hi], (hi == headers.size() - 1) ), opt.remove_overflow);
            data.insert(data.end(), buffer.begin(), buffer.end());
            if ( data.size() > 196608 ) {
                std::sort(data.begin(), data.end(), [](const word_t &lhs, const word_t &rhs) { return (double(rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });
                if ( opt.particle_gamma )
                    Event::BuildPGAndFill(std::vector<word_t>(data.begin(), data.begin()+65536), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
                else
                    Event::BuildAndFill(std::vector<word_t>(data.begin(), data.begin()+65536), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
                std::vector<word_t> new_data(data.begin()+65536, data.end());
                data.erase(data.begin(), data.begin()+65536);
            }
            if ( hi == headers.size() - 1 ){
                if ( opt.particle_gamma )
                    Event::BuildPGAndFill(std::vector<word_t>(data.begin(), data.end()), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
                else
                    Event::BuildAndFill(std::vector<word_t>(data.begin(), data.end()), &hm, ( opt.build_tree ) ? &tm : nullptr, ab_hist, opt.coincidence_time);
            }
            progress.UpdateReadProgress(hi);
        }
        progress.Finish();
    }
}


int main(int argc, char *argv[])
{
    CommandLineInterface interface;
    std::vector<std::string> input_file;
    std::string output_file, cal_file;//, sectBack_file, ringSect_file;
    Options opt = {1500, false, false, false, false, false};
    interface.Add("-i", "Input file", &input_file);
    interface.Add("-o", "Output file", &output_file);
    interface.Add("-c", "Calibration file", &cal_file);
    interface.Add("-ct", "Coincidence time", &opt.coincidence_time);
    interface.Add("-ab", "Addback", &opt.addback);
    interface.Add("-t", "Build tree", &opt.build_tree);
    interface.Add("-v", "Validate events", &opt.validate);
    //interface.Add("-sb", "SectBack gate", &sectBack_file);
    //interface.Add("-rs", "RingSect gate", &ringSect_file);
    interface.Add("-npg", "Not particle gamma event builder", &opt.particle_gamma);
    interface.Add("-nr", "Do not remove overflow events", &opt.remove_overflow);
    interface.CheckFlags(argc, argv);

    opt.particle_gamma = !opt.particle_gamma;
    opt.remove_overflow = !opt.remove_overflow;
    /*if ( sectBack_file != "")
        sectBackGate.Set(sectBack_file.c_str());
    if ( ringSect_file != "")
        ringSectGate.Set(ringSect_file.c_str());*/

    if (input_file.empty() || output_file.empty() ){
        std::cerr << "Input or output file missing." << std::endl;
        exit(EXIT_FAILURE);
    }

    if ( !cal_file.empty() ){
        if ( !SetCalibration(cal_file.c_str()) ){
            std::cerr << "Error reading calibration file." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    std::cout << "Running with options:" << std::endl;
    std::cout << opt << std::endl;

    //Convert_to_root_MT(input_file, output_file, opt);
    Convert_to_root_MM(input_file, output_file, opt);
    //Convert_to_root_MM_all(input_file, output_file, opt);
    exit(EXIT_SUCCESS);
}

