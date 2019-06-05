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

#include "PolygonGate.h"

#include <TROOT.h>

ProgressUI progress;
//PolygonGate sectBackGate;
//PolygonGate ringSectGate;

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

    Convert_to_root(input_file, output_file, opt);
    exit(EXIT_SUCCESS);
}

