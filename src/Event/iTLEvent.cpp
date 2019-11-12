//
// Created by Vetle Wegner Ingeberg on 11/11/2019.
//

#include <Parameters/experimentsetup.h>
#include <Parameters/Calibration.h>
#include <iostream>
#include <TH2.h>
#include <TTree.h>

#include "Event/iTLEvent.h"

using namespace Event;


bool iTLData::Add(const Parser::Entry_t &word)
{
    if ( mult < MAX_MULT ){
        ID[mult] = (GetDetector(word.address).type != clover) ?
                   GetDetector(word.address).detectorNum : GetDetector(word.address).detectorNum*NUM_CLOVER_CRYSTALS + GetDetector(word.address).telNum;
        e_raw[mult] = word.adcdata;
        energy[mult] = word.energy;
        tfine[mult] = word.cfdcorr;
        tcoarse[mult] = word.timestamp;
        cfdvalid[mult++] = !word.cfdfail;
        return true;
    }
    return false;
}

bool iTLData::Add(const iTLEntry &word)
{
    if ( mult < MAX_MULT ){
        ID[mult] = word.ID;
        e_raw[mult] = word.e_raw;
        energy[mult] = word.energy;
        tfine[mult] = word.tfine;
        tcoarse[mult] = word.tcoarse;
        cfdvalid[mult++] = word.cfdvalid;
        return true;
    }
    return false;
}


void iTLData::SetupBranch(TTree *tree, const char *baseName)
{
    char mult_name[2048], branch_name[2048], data_name[2048];
    sprintf(mult_name, "%sMult", baseName);
    sprintf(data_name, "%s/I", mult_name);
    bMult = tree->Branch(mult_name, &mult, data_name);
    sprintf(branch_name, "%sID", baseName);
    sprintf(data_name, "%s[%s]/s", branch_name, mult_name);
    bID = tree->Branch(branch_name, &ID, data_name);
    sprintf(branch_name, "%s_e_raw", baseName);
    sprintf(data_name, "%s[%s]/s", branch_name, mult_name);
    bRaw = tree->Branch(branch_name, &e_raw, data_name);
    sprintf(branch_name, "%sEnergy", baseName);
    sprintf(data_name, "%s[%s]/D", branch_name, mult_name);
    bEnergy = tree->Branch(branch_name, &energy, data_name);
    sprintf(branch_name, "%sTfine", baseName);
    sprintf(data_name, "%s[%s]/D", branch_name, mult_name);
    bTfine = tree->Branch(branch_name, &tfine, data_name);
    sprintf(branch_name, "%sTcoarse", baseName);
    sprintf(data_name, "%s[%s]/L", branch_name, mult_name);
    bTcoarse = tree->Branch(branch_name, &tcoarse, data_name);
    sprintf(branch_name, "%sCFDvalid", baseName);
    sprintf(data_name, "%s[%s]/O", branch_name, mult_name);
    bCfdvalid = tree->Branch(branch_name, &cfdvalid, data_name);
}


void iTLData::Copy(const Event::EventData *other)
{
    if ( other == nullptr )
        return;

    auto *iTLother = reinterpret_cast<const iTLData *>(other);
    mult = iTLother->mult;
    for ( int i = 0 ; i < iTLother->mult ; ++i ){
        ID[i] = iTLother->ID[i];
        e_raw[i] = iTLother->e_raw[i];
        energy[i] = iTLother->energy[i];
        tfine[i] = iTLother->tfine[i];
        tcoarse[i] = iTLother->tcoarse[i];
        cfdvalid[i] = iTLother->cfdvalid[i];
    }
}

std::vector<iTLEntry> iTLData::GetEntries() const
{
    std::vector<iTLEntry> data(mult);
    for ( int i = 0 ; i < mult ; ++i ){
        data.push_back({ID[i], e_raw[i], energy[i], tfine[i], tcoarse[i], cfdvalid[i]});
    }
    return data;
}


iTLEvent::iTLEvent(TTree *tree)
{
    Register(&ringData, "ring");
    Register(&sectData, "sector");
    Register(&backData, "back");
    Register(&labrLData, "labrL");
    Register(&labrSData, "labrS");
    Register(&labrFData, "labrF");
    Register(&cloverData, "clover");
    Register(&rfData, "rf");
    if ( tree != nullptr )
        SetupTree(tree);
}

iTLEvent::iTLEvent(const std::vector<Parser::Entry_t> &data)
        : iTLEvent()
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

void iTLEvent::Addback(TH2 *ab_t_clover)
{
    // We set up a vector for each clover.
    std::vector<iTLEntry> cevent[NUM_CLOVER_DETECTORS];
    for ( auto &entry : cloverData.GetEntries() ){
        cevent[entry.ID/4].push_back(entry);
    }
    cloverData.Reset();
    std::vector<iTLEntry> v, v_new;
    double e, tdiff;
    for (size_t n = 0 ; n < NUM_CLOVER_DETECTORS ; ++n){
        v = cevent[n];
        std::sort(v.begin(), v.end(), [](const iTLEntry &lhs, const iTLEntry &rhs){ return lhs.energy > rhs.energy; });
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
            cloverData.Add(iTLEntry({v[0].ID, 0, e, v[0].tfine, v[0].tcoarse, v[0].cfdvalid}));
            v = v_new;
        }
    }
}