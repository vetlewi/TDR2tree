//
// Created by Vetle Wegner Ingeberg on 01/03/2022.
//

#include "RFTreeEvent.h"
#include "Calibration.h"

#include "BasicStruct.h"
#include "Event.h"

struct cfd_decode {
    unsigned val : 13;
    unsigned cross : 3;
};

RFTreeData::RFTreeData(TTree *tree, const char *name)
        : entries( 0 )
        , entries_branch( tree->Branch(CName(name, "Mult").c_str(), &entries, std::string(CName(name, "Mult") + "/I").c_str()) )
        , ID( tree, CName(name, "ID").c_str(),  CLeaf(name, "ID", type_map::unsigned_short_type).c_str() )
        , veto( tree, CName(name, "Veto").c_str(), CLeaf(name, "Veto", type_map::bool_type).c_str() )
        , cfd_fail( tree, CName(name, "CFDfail").c_str(), CLeaf(name, "CFDfail", type_map::bool_type).c_str() )
        , energy( tree, CName(name, "Energy").c_str(), CLeaf(name, "Energy", type_map::double_type).c_str() )
        , time(tree, CName(name, "Time").c_str(), CLeaf(name, "Time", type_map::double_type).c_str() )
        , raw_cfd(tree, CName(name, "CFDraw").c_str(), CLeaf(name, "CFDraw", type_map::unsigned_short_type).c_str() )
        , cfd_cross(tree, CName(name, "CFDcross").c_str(), CLeaf(name, "CFDcross", type_map::unsigned_short_type).c_str() )
        , cfd_val(tree, CName(name, "CFDval").c_str(), CLeaf(name, "CFDval", type_map::unsigned_short_type).c_str() )
{}

void RFTreeData::push_back(const word_t &event, const word_t &rfevent)
{
    auto *detector = GetDetectorPtr(event.address);
    ID.push_back(( detector->type == clover ) ? detector->detectorNum*NUM_CLOVER_CRYSTALS + detector->telNum : detector->detectorNum, entries );
    veto.push_back(event.veto, entries);
    cfd_fail.push_back(event.cfdfail, entries);
    energy.push_back(event.energy, entries);
    double timediff = double(event.timestamp - rfevent.timestamp) + event.cfdcorr - rfevent.cfdcorr;
    uint16_t cfdvalue = event.cfddata;

    if ( detector->sfreq == f500MHz && detector->type != rfchan && !event.cfdfail ){
        // We add 3072. If overflow then we will need to subtract 10 ns from the time
        cfdvalue = event.cfddata + 3500;
        // If we become larger
        auto cfdcoded_old = reinterpret_cast<const cfd_decode *>(&event.cfddata);
        auto cfdcoded = reinterpret_cast<cfd_decode *>(&cfdvalue);

        if ( cfdcoded->cross > 4 && reinterpret_cast<const cfd_decode *>(&rfevent.cfddata)->cross == 0 ){ // This actually means overflow.
            //cfdcoded->cross = 0; // Manually set the cross value to ensure that we got an actual overflow
            //timediff -= 10;
        }

        // Re-evaluate the cfd correction
        double cfdcorr = (cfdcoded->cross - 1 + cfdcoded->val/8192.) * 2;
        cfdcorr += CalibrateTime(event);
        //timediff = double(event.timestamp - rfevent.timestamp) +
        //cfdvalue = event.cfddata;
        //if ( cfdvalue < event.cfddata )
        //    timediff -= 10;
    }

    time.push_back(timediff, entries);
    raw_cfd.push_back(cfdvalue, entries);
    auto cfdcoded = reinterpret_cast<const cfd_decode *>(&cfdvalue);
    cfd_cross.push_back(cfdcoded->cross, entries);
    cfd_val.push_back(cfdcoded->val, entries);
    ++entries;
}

RFTreeEvent::RFTreeEvent(TTree *tree)
        : clover(tree, "clover")
        , labrL(tree, "labrL")
        , labrS(tree, "labrS")
        , labrF(tree, "labrF")
        , ring(tree, "ring")
        , sect(tree, "sect")
        , back(tree, "back")
        , rf(tree, "rf")
{}

RFTreeData &RFTreeEvent::GetData(const DetectorType &type)
{
    switch ( type ) {
        case DetectorType::clover : return clover;
        case labr_3x8 : return labrL;
        case labr_2x2_ss : return labrS;
        case labr_2x2_fs : return labrF;
        case de_ring : return ring;
        case de_sect : return sect;
        case eDet : return back;
        case rfchan : return rf;
        default: return clover;
    }
}

RFTreeEvent &RFTreeEvent::operator=(Event &event)
{
    // First we will clear everything
    clear();

    // Get the RF entry
    auto rfentries = event.GetDetector(rfchan);
    //assert(rfentries.size() == 0);
    if ( rfentries.size() != 1 ){
        // Should be some kind of error...
        // For now we will do nothing :-/
    }


    // Then we will go type by type and add events
    for ( auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, DetectorType::clover} ){
        auto data = event.GetDetector(type);
        GetData(type).add(data.begin(), data.end(), *rfentries.begin());
    }
    return *this;
}

void RFTreeEvent::push_back(const word_t &event, const word_t &rfevent)
{
    switch ( GetDetectorPtr(event.address)->type ) {
        case DetectorType::clover :
            clover.push_back(event, rfevent);
            break;
        case DetectorType::labr_3x8 :
            labrL.push_back(event, rfevent);
            break;
        case DetectorType::labr_2x2_ss :
            labrS.push_back(event, rfevent);
            break;
        case DetectorType::labr_2x2_fs :
            labrF.push_back(event, rfevent);
            break;
        case DetectorType::de_ring :
            ring.push_back(event, rfevent);
            break;
        case DetectorType::de_sect :
            sect.push_back(event, rfevent);
            break;
        case DetectorType::eDet :
            back.push_back(event, rfevent);
            break;
        case DetectorType::rfchan :
            rf.push_back(event, rfevent);
        default:
            break;
    }
}