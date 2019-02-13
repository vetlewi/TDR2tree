#include "tools.h"

#include "FileReader.h"
#include "experimentsetup.h"
#include "Calibration.h"
#include "BasicStruct.h"

#include <iostream>
#include <cmath>
#include <forward_list>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>

const int buf_size = 32768;


std::vector<word_t> ReadFileToMemory(const char *filename)
{
    FileReader freader;
    freader.Open(filename);
    int i;
    std::vector<word_t> data;
    data.reserve(128876544);

    // We read 32k at the time
    word_t buf[buf_size];
    while (freader.Read(buf,buf_size) > 0){
        for (i = 0 ; i < buf_size ; ++i){
            data.push_back(buf[i]);
        }
    }
    return data;
}


//! Function to setup branches in the tree. Maybe this should be moved to the event structure?
void SetupBranches(Event &eventstr, TTree *tree)
{
    // Setup rings
    tree->Branch("ring_mult", &eventstr.ring_mult, "ring_mult/S");
    tree->Branch("ringID", &eventstr.ringID, "ringID[ring_mult]/S");
    tree->Branch("ring_energy", &eventstr.ring_energy, "ring_energy[ring_mult]/D");
    tree->Branch("ring_t_fine", &eventstr.ring_t_fine, "ring_t_fine[ring_mult]/D");
    tree->Branch("ring_t_course",&eventstr.ring_t_course, "ring_t_course[ring_mult]/L");

    // Setup sectors
    tree->Branch("sect_mult", &eventstr.sect_mult, "sect_mult/S");
    tree->Branch("sectID", &eventstr.sectID, "sectID[sect_mult]/S");
    tree->Branch("sect_energy", &eventstr.sect_energy, "sect_energy[sect_mult]/D");
    tree->Branch("sect_t_fine", &eventstr.sect_t_fine, "sect_t_fine[sect_mult]/D");
    tree->Branch("sect_t_course", &eventstr.sect_t_course, "sect_t_course[sect_mult]/L");

    // Setup back
    tree->Branch("back_mult", &eventstr.back_mult, "back_mult/S");
    tree->Branch("backID", &eventstr.backID, "backID[back_mult]/S");
    tree->Branch("back_energy", &eventstr.back_energy, "back_energy[back_mult]/D");
    tree->Branch("back_t_fine", &eventstr.back_t_fine, "back_t_fine[back_mult]/D");
    tree->Branch("back_t_course", &eventstr.back_t_course, "back_t_course[back_mult]/L");

    // Setup labr L
    tree->Branch("labrL_mult", &eventstr.labrL_mult, "labrL_mult/S");
    tree->Branch("labrLID", &eventstr.labrLID, "labrLID[labrL_mult]/S");
    tree->Branch("labrL_energy", &eventstr.labrL_energy, "labrL_energy[labrL_mult]/D");
    tree->Branch("labrL_t_fine", &eventstr.labrL_t_fine, "labrL_t_fine[labrL_mult]/D");
    tree->Branch("labrL_t_course", &eventstr.labrL_t_course, "labrL_t_course[labrL_mult]/L");

    // Setup labr S
    tree->Branch("labrS_mult", &eventstr.labrS_mult, "labrS_mult/S");
    tree->Branch("labrSID", &eventstr.labrSID, "labrSID[labrS_mult]/S");
    tree->Branch("labrS_energy", &eventstr.labrS_energy, "labrS_energy[labrS_mult]/D");
    tree->Branch("labrS_t_fine", &eventstr.labrS_t_fine, "labrS_t_fine[labrS_mult]/D");
    tree->Branch("labrS_t_course", &eventstr.labrS_t_course, "labrS_t_course[labrS_mult]/L");

    // Setup clover
    tree->Branch("clover_mult",&eventstr.clover_mult, "clover_mult/S");
    tree->Branch("cloverID",&eventstr.cloverID, "cloverID[clover_mult]/S");
    tree->Branch("clover_crystal",&eventstr.clover_crystal, "clover_crystal[clover_mult]/S");
    tree->Branch("clover_energy", &eventstr.clover_energy, "clover_energy[clover_mult]/D");
    tree->Branch("clover_t_fine", &eventstr.clover_t_fine, "clover_t_fine[clover_mult]/D");
    tree->Branch("clover_t_course", &eventstr.clover_t_course, "clover_t_course[clover_mult]/L");
    tree->BranchRef();
}


void Convert_file(const std::string in_name, TTree *tree, HistogramManager *mgr, Event *eventstr, const Options &opt)
{
    int64_t timediff;
    size_t start=0,stop=0;
    size_t i, j;
    DetectorInfo_t trigger, channel;


    int barWidth = 50;
    int shown = 0;

    std::cout << "[";
    for (int p = 0 ; p < barWidth ; ++p){
        std::cout << " ";
    }
    std::cout << "] " << 0 << "% Reading from file '" << in_name << "'" << "\r";
    std::cout.flush();
    std::vector<word_t> full_file = ReadFileToMemory(in_name.c_str());

    // Build events
    for (i = 0 ; i < full_file.size() ; ++i){
        trigger = GetDetector(full_file[i].address);

        if ( trigger.type != de_ring ) // Skip to next word.
            continue;

        for (j = i ; j > 0 ; --j){
            timediff = abs(full_file[i].timestamp - full_file[j-1].timestamp);
            if (timediff > 1500){
                start = j;
                break;
            }
        }

        for (j = i ; j < full_file.size() - 1 ; ++j){
            timediff = abs(full_file[i].timestamp - full_file[j+1].timestamp);
            if (timediff > 1500){
                stop = j+1;
                break;
            }
        }

        eventstr->Reset();

        eventstr->AddRing(trigger.detectorNum, CalibrateEnergy(full_file[i]), full_file[i].timestamp, CalibrateTime(full_file[i]));

        for (j = start ; j < stop ; ++j){
            if ( j == i )
                continue;
            channel = GetDetector(full_file[j].address);
            switch (channel.type) {
                case de_sect : {
                    eventstr->AddSect(channel.detectorNum, CalibrateEnergy(full_file[j]), full_file[j].timestamp, CalibrateTime(full_file[j]));
                    break;
                }
                case eDet : {
                    eventstr->AddBack(channel.detectorNum, CalibrateEnergy(full_file[j]), full_file[j].timestamp, CalibrateTime(full_file[j]));
                    break;
                }
                case labr_3x8 : {
                    eventstr->AddLabrL(channel.detectorNum, CalibrateEnergy(full_file[j]), full_file[j].timestamp, CalibrateTime(full_file[j]));
                    break;
                }
                case labr_2x2 : {
                    eventstr->AddLabrS(channel.detectorNum, CalibrateEnergy(full_file[j]), full_file[j].timestamp, CalibrateTime(full_file[j]));
                    break;
                }
                case clover : {
                    if ( full_file[j].adcdata <= 16382 )
                        eventstr->AddClover(channel.detectorNum*4 + channel.telNum, CalibrateEnergy(full_file[j]), full_file[j].timestamp, CalibrateTime(full_file[j]));
                    break;
                }
                default :
                    break;
            }
        }
        mgr->Fill(eventstr, opt);
        if (opt.use_addback)
            eventstr->RunAddback(mgr->GetAB());
        if( (i / double(full_file.size())) - shown*0.02 > 0 ){
            if ( tree != nullptr) tree->OptimizeBaskets();
            ++shown;
            std::cout << "[";
            int pos = barWidth * i / double(full_file.size());
            for (int p = 0 ; p < barWidth ; ++p){
                if (p < pos) std::cout << "=";
                else if (p == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(100 * (i / double(full_file.size()) )) << "% Processing file '" << in_name << "'\r";
            std::cout.flush();
        }
        if (tree != nullptr) tree->Fill();
    }
    std::cout << "[";
    for (int p = 0 ; p < barWidth ; ++p){
        std::cout << "=";
    }
    std::cout << "] " << int(100 * (i / double(full_file.size()) )) << "% Done processing file '" << in_name << "'" << std::endl;
}

void Convert_to_ROOT(const std::vector<std::string> &in_names, const char *out_name,  const Options &opt)
{
    TFile *fout = new TFile(out_name, "RECREATE");
    TTree *tree = nullptr;
    Event eventstr;

    if (opt.make_tree){
        tree = new TTree("events","events");
        SetupBranches(eventstr, tree);
    }

    HistogramManager hmg;

    // We will read file by file.
    // Make a copy of in_names since someone is messing with it :(
    std::vector<std::string> fnames(in_names.size());
    for (size_t i = 0 ; i < in_names.size() ; ++i){
        fnames.push_back(in_names[i]);
    }

    for (size_t i = 0 ; i < in_names.size() ; ++i){
        if (opt.make_tree) {
            Convert_file(in_names[i].c_str(), tree, &hmg, &eventstr, opt);
        } else {
            Convert_file(in_names[i].c_str(), nullptr, &hmg, &eventstr, opt);
        }
    }
    fout->Write();
    fout->Close();
}
