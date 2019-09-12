#include <iostream>

#include <string>
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
#include "STFileBufferFetcher.h"
#include "BufferType.h"
#include "TDRFileReader.h"

#include <TROOT.h>

ProgressUI progress;

struct Options {
    int coincidence_time;
    bool build_tree;
    bool particle_gamma;
    bool addback;
    bool validate;

};

std::ostream &operator<<(std::ostream &os, const Options &opt)
{
    os << "Coincidence time: " << opt.coincidence_time;
    os << "\nAddback: " << ( opt.addback ? "true" : "false");
    os << "\nParticle gamma: " << ( opt.particle_gamma ? "true" : "false");
    os << "\nBuild tree: " << ( opt.build_tree ? "true" : "false");
    os << "\nValidate: " << ( opt.validate ? "true " : "false");
    return os;
}

/*void Convert_to_root(const std::vector<std::string> &in_files, const std::string &out_file, const Options &opt)
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
}*/

template<class T>
struct ROOT_objects {
    RootFileManager *fm;
    HistManager *hm;
    TreeManager<T> *tm;
};

template<typename T, int BufferSize = 65536>
class BufferedEvents {

private:

    //! Fetcher object.
    BufferFetcher *fetcher;

    //! Object to store the buffered data.
    std::vector<T> buffer_data;

    //! Method to add data to the buffer.
    void AddData(const T *ptr, const int &size)
    {
        int set = 0;
        while ( set < size ){
            const T *d = reinterpret_cast<const T *>(ptr+set++);
            buffer_data.push_back(*d);
        }
    }

public:

    //! initializer
    BufferedEvents(BufferFetcher *fetch) : fetcher( fetch ), buffer_data( 2*BufferSize ){ buffer_data.clear(); }

    //! Get a buffer of data.
    std::vector<T> GetData(BufferFetcher::Status &status)
    {
        const Buffer *buffer = fetcher->Next(status);
        if (status == BufferFetcher::OKAY) {
            int bufSize = buffer->GetSize();
            AddData(reinterpret_cast<const T *>(buffer->CGetBuffer()), bufSize / sizeof(T));
        } else {
            return std::vector<T>(0);
        }
        std::sort(std::begin(buffer_data), std::end(buffer_data));
        if (buffer_data.size() >= 2 * BufferSize) {
            std::vector<T> res(2*BufferSize);
            std::move(std::begin(buffer_data), std::end(buffer_data) - BufferSize, res.begin());
            buffer_data.erase(buffer_data.begin(), buffer_data.end() - BufferSize);
            return res;
        } else {
            return GetData(status);
        }
    }

    //! Flush the buffer.
    std::vector<T> Flush(){ return buffer_data; }

};

template<typename EventType, typename EntryType>
class RootFileConverter
{
private:

    //! Class responsible for fetching data from file.
    aptr<FileBufferFetcher> file_fetcher;

    //! Options for the event building, etc.
    Options options;

    //! Class handling I/O control.
    RootFileManager fileManager;

    //! Class handling filling of histograms.
    HistManager histManager;

    //! Class handling filling of trees.
    TreeManager<EventType> treeManager;

    //! Class buffering events.
    BufferedEvents<EntryType> buffer_event_fetcher;

    //! Addback spectra.
    TH2 *ab_hist;

public:

    //! Initializer
    RootFileConverter(FileBufferFetcher *fetcher, const char *out_file, const Options &opt)
        : file_fetcher( fetcher )
        , options( opt )
        , fileManager( out_file )
        , histManager( &fileManager )
        , treeManager( &fileManager, "events", "Event tree", opt.validate )
        , buffer_event_fetcher( file_fetcher.get() )
        , ab_hist(  (options.addback) ?
                    fileManager.CreateTH2("time_self_clover", "Time spectra, clover self timing",
                                          3000, -1500, 1500, "Time [ns]",
                                          NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector")
                                          : nullptr ){}



    //! Extract and sort files.
    bool ConvertFiles(const std::vector<std::string> &files);

};

template<typename EventType, typename EntryType>
bool RootFileConverter<EventType, EntryType>::ConvertFiles(const std::vector<std::string> &files)
{
    BufferFetcher::Status status;
    for ( auto &file : files ){
        status = file_fetcher->Open(file.c_str(), 0);
        if ( status != BufferFetcher::OKAY ){
            std::cerr << __PRETTY_FUNCTION__ << ": Unable to open file '" << file << "', skipping..." << std::endl;
            continue;
        }


        while ( status == BufferFetcher::OKAY ){

            std::vector<EntryType> events = buffer_event_fetcher.GetData(status);

            if ( status == BufferFetcher::END )
                break;

            if ( options.build_tree ){
                Event::BuildPGAndFill(events, &histManager,( options.build_tree ) ? &treeManager : nullptr,
                                      ab_hist, options.coincidence_time);
            } else {
                Event::BuildAndFill(events, &histManager,( options.build_tree ) ? &treeManager : nullptr,
                                    ab_hist, options.coincidence_time);
            }
        }
    }

    if ( status == BufferFetcher::END ){
        std::vector<EntryType> events = buffer_event_fetcher.Flush();

        if ( options.build_tree ){
            Event::BuildPGAndFill(events, &histManager,( options.build_tree ) ? &treeManager : nullptr,
                                  ab_hist, options.coincidence_time);
        } else {
            Event::BuildAndFill(events, &histManager,( options.build_tree ) ? &treeManager : nullptr,
                                ab_hist, options.coincidence_time);
        }
    }

    return true;
}


int main(int argc, char *argv[])
{
    CommandLineInterface interface;
    std::vector<std::string> input_file;
    std::string output_file, cal_file;//, sectBack_file, ringSect_file;
    Options opt = {1500, false, false, false, false};

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
    interface.CheckFlags(argc, argv);

    opt.particle_gamma = !opt.particle_gamma;
    /*if ( sectBack_file != "")
        sectBackGate.Set(sectBack_file.c_str());
    if ( ringSect_file != "")
        ringSectGate.Set(ringSect_file.c_str());*/

    if (input_file.empty() || output_file.empty() ){
        std::cerr << "Input or output file missing." << std::endl;
        return -1;
    }

    if ( !cal_file.empty() ){
        if ( !SetCalibration(cal_file.c_str()) ){
            std::cerr << "Error reading calibration file." << std::endl;
            return -1;
        }
    }

    std::cout << "Running with options:" << std::endl;
    std::cout << opt << std::endl;
    FileBufferFetcher *bufFetch = new STFileBufferFetcher(new TDRFileReader());
    RootFileConverter<Event, word_t> converter(bufFetch, output_file.c_str(), opt);

    converter.ConvertFiles(input_file);
    exit(EXIT_SUCCESS);
}

