//
// Created by Vetle Wegner Ingeberg on 16/09/2019.
//

#include "TDREvent.h"
#include "experimentsetup.h"
#include "Calibration.h"

#include <vector>
#include <string>
#include <iostream>
#include <TTree.h>
#include <TH2.h>

using namespace Event;

void TDREventData::SetupBranch(TTree *tree, const char *baseName)
{
    char mult_name[2048], branch_name[2048], data_name[2048];
    sprintf(mult_name, "%sMult", baseName);
    sprintf(data_name, "%s/I", mult_name);
    tree->Branch(mult_name, &mult, data_name);
    sprintf(branch_name, "%sID", baseName);
    sprintf(data_name, "%s[%s]/s", branch_name, mult_name);
    tree->Branch(branch_name, &ID, data_name);
    sprintf(branch_name, "%sEnergy", baseName);
    sprintf(data_name, "%s[%s]/D", branch_name, mult_name);
    tree->Branch(branch_name, &energy, data_name);
    sprintf(branch_name, "%sTfine", baseName);
    sprintf(data_name, "%s[%s]/D", branch_name, mult_name);
    tree->Branch(branch_name, &tfine, data_name);
    sprintf(branch_name, "%sTcoarse", baseName);
    sprintf(data_name, "%s[%s]/L", branch_name, mult_name);
    tree->Branch(branch_name, &tcoarse, data_name);
}


bool TDREventData::Add(const word_t &word)
{
    if ( mult < MAX_NUM ){
        ID[mult] = (GetDetector(word.address).type != clover) ?
                GetDetector(word.address).detectorNum
                : GetDetector(word.address).detectorNum*NUM_CLOVER_CRYSTALS + GetDetector(word.address).telNum;
        e_raw[mult] = word.adcdata;
        energy[mult] = word.energy;
        tfine[mult] = word.cfdcorr;
        tcoarse[mult] = word.timestamp;
        cfdvalid[mult++] = !word.cfdfail;
        return true;
    }
    return false;
}

bool TDREventData::Add(const TDREntry &entry)
{
    if ( mult < MAX_NUM ){
        ID[mult] = entry.ID;
        e_raw[mult] = entry.e_raw;
        energy[mult] = entry.energy;
        tfine[mult] = entry.tfine;
        tcoarse[mult] = entry.tcoarse;
        cfdvalid[mult++] = entry.cfdvalid;
        return true;
    }
    return false;
}


void TDRTimeData::SetupBranch(TTree *tree, const char *baseName)
{
    char mult_name[2048], branch_name[2048], data_name[2048];
    sprintf(mult_name, "%sMult", baseName);
    sprintf(data_name, "%s/I", mult_name);
    tree->Branch(mult_name, &mult, data_name);
    sprintf(branch_name, "%sTfine", baseName);
    sprintf(data_name, "%s[%s]/D", branch_name, mult_name);
    tree->Branch(branch_name, &tfine, data_name);
    sprintf(branch_name, "%sTcoarse", baseName);
    sprintf(data_name, "%s[%s]/L", branch_name, mult_name);
    tree->Branch(branch_name, &tcoarse, data_name);
}

bool TDRTimeData::Add(const word_t &word)
{
    if ( mult < MAX_NUM ){
        tfine[mult] = word.cfdcorr;
        tcoarse[mult] = word.timestamp;
        cfdvalid[mult++] = !word.cfdfail;
        return true;
    }
    return false;
}

bool TDRTimeData::Add(const TDREntry &entry)
{
    if ( mult < MAX_NUM ){
        tfine[mult] = entry.tfine;
        tcoarse[mult] = entry.tcoarse;
        cfdvalid[mult++] = entry.cfdvalid;
        return true;
    }
    return false;
}


TDREvent::TDREvent()
{
    Register(&ringData, "ring");
    Register(&sectData, "sector");
    Register(&backData, "back");
    Register(&labrLData, "labrL");
    Register(&labrSData, "labrS");
    Register(&labrFData, "labrF");
    Register(&cloverData, "clover");
    Register(&rfData, "rf");
}

TDREvent::TDREvent(const std::vector<word_t> &data)
    : TDREvent()
{
    for ( auto &entry : data ){

        switch ( GetDetectorType(entry.address) ){

            case de_ring : {
                if ( !ringData.Add(entry) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many ring words." << std::endl;
                }
                break;
            }

            case de_sect : {
                if ( !sectData.Add(entry) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many sect words." << std::endl;
                }
                break;
            }

            case eDet : {
                if ( !backData.Add(entry) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many back words." << std::endl;
                }
                break;
            }

            case labr_3x8 : {
                if ( !labrLData.Add(entry) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrL words." << std::endl;
                }
                break;
            }

            case labr_2x2_ss : {
                if ( !labrSData.Add(entry) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrS words." << std::endl;
                }
                break;
            }

            case labr_2x2_fs : {
                if ( !labrFData.Add(entry) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrF words." << std::endl;
                }
                break;
            }

            case clover : {
                if ( !cloverData.Add(entry) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many clover words." << std::endl;
                }
                break;
            }

            case rfchan : {
                if ( !rfData.Add(entry) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many rf words." << std::endl;
                }
                break;
            }
            default :
                break;
        }
    }
}

void TDREvent::Addback(TH2 *ab_t_clover)
{
    // We set up a vector for each clover.
    std::vector<TDREntry> cevent[NUM_CLOVER_DETECTORS];
    for ( auto &entry : cloverData.GetEntries() ){
        cevent[entry.ID/4].push_back(entry);
    }
    cloverData.Reset();
    std::vector<TDREntry> v, v_new;
    double e, tdiff;
    for (size_t n = 0 ; n < NUM_CLOVER_DETECTORS ; ++n){
        v = cevent[n];
        std::sort(v.begin(), v.end(), [](const TDREntry &lhs, const TDREntry &rhs){ return lhs.energy > rhs.energy; });
        while ( !v.empty() ){
            v_new.clear();
            e = v[0].energy;
            for (size_t m = 1 ; m < v.size() ; ++m){
                tdiff = double(v[m].tcoarse - v[0].tcoarse);
                tdiff += (v[m].tfine - v[0].tfine);
                ab_t_clover->Fill(tdiff, n);
                if ( CheckTimeGateAddback(tdiff) ){
                    e += v[m].energy;
                } else {
                    v_new.push_back(v[m]);
                }
            }
            cloverData.Add(TDREntry({v[0].ID, 0, e, v[0].tfine, v[0].tcoarse, v[0].cfdvalid}));
            v = v_new;
        }
    }
}