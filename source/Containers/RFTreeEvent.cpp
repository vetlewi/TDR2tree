//
// Created by Vetle Wegner Ingeberg on 01/03/2022.
//

#include "RFTreeEvent.h"

#include "BasicStruct.h"
#include "Event.h"

enum type_map : char {
    char_array_type = 'C',
    signed_char_type = 'B',
    unsigned_char_type = 'b',
    signed_short_type = 'S',
    unsigned_short_type = 's',
    signed_integer_type = 'I',
    unsigned_integer_type = 'i',
    signed_long_type = 'L',
    unsigned_long_type = 'l',
    float_type = 'F',
    double_type = 'D',
    bool_type = 'O'
};

std::string CName(const char *base_name, const char *identifier)
{
    return std::string(base_name) + std::string(identifier);
}

std::string CLeaf(const char *base_name, const char *identifier, const char type)
{
    return CName(base_name, identifier) + "[" + CName(base_name, "Mult") + "]/" + std::string(1, type);
}

RFTreeData::RFTreeData(TTree *tree, const char *name)
        : entries( 0 )
        , entries_branch( tree->Branch(CName(name, "Mult").c_str(), &entries, std::string(CName(name, "Mult") + "/I").c_str()) )
        , ID( tree, CName(name, "ID").c_str(),  CLeaf(name, "ID", type_map::unsigned_short_type).c_str() )
        , veto( tree, CName(name, "Veto").c_str(), CLeaf(name, "Veto", type_map::bool_type).c_str() )
        , cfd_fail( tree, CName(name, "CFDfail").c_str(), CLeaf(name, "CFDfail", type_map::bool_type).c_str() )
        , energy( tree, CName(name, "Energy").c_str(), CLeaf(name, "Energy", type_map::double_type).c_str() )
        , time(tree, CName(name, "Time").c_str(), CLeaf(name, "Time", type_map::double_type).c_str() )
{}

void RFTreeData::push_back(const word_t &event, const word_t &rfevent)
{
    auto *detector = GetDetectorPtr(event.address);
    ID.push_back(( detector->type == clover ) ? detector->detectorNum*NUM_CLOVER_CRYSTALS + detector->telNum : detector->detectorNum, entries );
    veto.push_back(event.veto, entries);
    cfd_fail.push_back(event.cfdfail, entries);
    energy.push_back(event.energy, entries);
    time.push_back(event.timestamp, entries);
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
        default: return clover;
    }
}

RFTreeEvent &RFTreeEvent::operator=(Event &event)
{
    // First we will clear everything
    clear();

    // Get the RF entry
    auto rfentries = event.GetDetector(rfchan);
    assert(rfentries.size() == 0);
    if ( rfentries.size() != 1 ){
        // Should be some kind of error...
        // For now we will do nothing :-/
    }


    // Then we will go type by type and add events
    for ( auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, DetectorType::clover} ){
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
        default:
            break;
    }
}