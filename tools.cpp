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


/*void Convert_file(const std::string in_name, TTree *tree, HistogramManager *mgr, Event *eventstr, const Options &opt)
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

        if ( trigger.type != eDet ) // Skip to next word.
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

        eventstr->AddBack(trigger.detectorNum, CalibrateEnergy(full_file[i]), full_file[i].timestamp, CalibrateTime(full_file[i]));

        for (j = start ; j < stop ; ++j){
            if ( j == i )
                continue;
            channel = GetDetector(full_file[j].address);
            switch (channel.type) {
                case de_ring : {
                    eventstr->AddRing(channel.detectorNum, CalibrateEnergy(full_file[j]), full_file[j].timestamp, CalibrateTime(full_file[j]));
                    break;
                }
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
                case labr_2x2_ss : {
                    eventstr->AddLabrS(channel.detectorNum, CalibrateEnergy(full_file[j]), full_file[j].timestamp, CalibrateTime(full_file[j]));
                    break;
                }
                case labr_2x2_fs : {
                    eventstr->AddLabrF(channel.detectorNum, CalibrateEnergy(full_file[j]), full_file[j].timestamp, CalibrateTime(full_file[j]));
                    break;
                }
                case rfchan : {
                    eventstr->AddRF(full_file[j].timestamp, full_file[j].cfdcorr);
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
            if ( opt.make_tree ) tree->OptimizeBaskets();
            ++shown;
            std::cout << "[";
            int pos = int( barWidth * i / double(full_file.size()) );
            for (int p = 0 ; p < barWidth ; ++p){
                if (p < pos) std::cout << "=";
                else if (p == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int(100 * (i / double(full_file.size()) )) << "% Processing file '" << in_name << "'\r";
            std::cout.flush();
        }
        if ( opt.make_tree ) tree->Fill();
    }
    std::cout << "[";
    for (int p = 0 ; p < barWidth ; ++p){
        std::cout << "=";
    }
    std::cout << "] " << int(100 * (i / double(full_file.size()) )) << "% Done processing file '" << in_name << "'" << std::endl;
}*/

void Convert_to_ROOT(const std::vector<std::string> &in_names, const char *out_name/*,  const Options &opt*/)
{
    return;
}
/*{
    TFile *fout = new TFile(out_name, "RECREATE");
    TTree *tree = nullptr;
    Event eventstr;

    if (opt.make_tree){
        tree = new TTree("events","events");
        eventstr.SetupBranches(tree);
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
}*/
