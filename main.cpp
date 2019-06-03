#include <iostream>

#include <string>
#include <vector>

#include "CommandLineInterface.h"
#include "Calibration.h"
#include "experimentsetup.h"

#include "RootFileManager.h"
#include "HistManager.h"
#include "TreeManager.h"
#include "Event.h"
#include "ProgressUI.h"

#include <TROOT.h>

ProgressUI progress;


void Convert_to_root(const std::vector<std::string> &in_files, const std::string &out_file, const int &coins_t, const bool &buildTree=true, const bool &pg_event=true, const bool &ab=true)
{
    std::vector<Event> event_data;
    RootFileManager fm(out_file.c_str());
    HistManager hm(&fm);
    TreeManager<Event> tm(&fm, "events", "Event tree");
    TH2 *ab_hist = ( ab ) ? fm.CreateTH2("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") : nullptr;
    for ( const auto& file : in_files ) {
        /*EventBuilder builder(FileReader::GetFile(file.c_str()), ab_hist);
        code_machina::BlockingCollection<Event> collection(16384);
        std::thread consumer([&collection, &hm, &tm](){
            int events_added = 0;
            while ( !collection.is_completed() ){
                Event ev;
                auto status = collection.take(ev);
                if ( status == code_machina::BlockingCollectionStatus::Ok ){
                    hm.AddEntry(ev);
                    tm.AddEntry(ev);
                    ++events_added;
                }
            }
        });

        std::thread producer([&collection, &builder](){
            Event ev;
            while( builder.GetEvent(ev) ){
                collection.add(ev);
            }
            collection.complete_adding();
        });

        producer.join();
        consumer.join();*/

        //event_data = ( pg_event ) ? Event::BuildPGEvents(FileReader::GetFile(file.c_str()), ab_hist) : Event::BuildEvent(FileReader::GetFile(file.c_str()), ab_hist);
        //hm.AddEntries(event_data);
        //tm.AddEntries(event_data);
        if ( pg_event )
            Event::BuildPGAndFill(FileReader::GetFile(file.c_str()), &hm, ( buildTree ) ? &tm : nullptr, ab_hist, coins_t);
        else
            Event::BuildAndFill(FileReader::GetFile(file.c_str()), &hm, ( buildTree ) ? &tm : nullptr, ab_hist, coins_t);
        progress.Finish();
    }
}


int main(int argc, char *argv[])
{
    CommandLineInterface interface;
    std::vector<std::string> input_file;
    std::string output_file, cal_file;
    bool npg, ab, bt;
    int coins_time = 3000;

    interface.Add("-i", "Input file", &input_file);
    interface.Add("-o", "Output file", &output_file);
    interface.Add("-ct", "Coincidence time", &coins_time);
    interface.Add("-c", "Calibration file", &cal_file);
    interface.Add("-ab", "Addback", &ab);
    interface.Add("-t", "Build tree", &bt);
    interface.Add("-npg", "Not particle gamma event builder", &npg);
    interface.CheckFlags(argc, argv);

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

    Convert_to_root(input_file, output_file, coins_time, bt, !npg, ab);


    //Convert_to_ROOT(input_file, output_file.c_str(), opt);
    return 0;

}

