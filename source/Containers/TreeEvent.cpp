//
// Created by Vetle Wegner Ingeberg on 15/04/2021.
//


#include "TreeEvent.h"

#include "experimentsetup.h"
#include "BasicStruct.h"

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

TreeData::TreeData(TTree *tree, const char *name)
    : entries( 0 )
    , entries_branch( tree->Branch(CName(name, "Mult").c_str(), &entries, std::string(CName(name, "Mult") + "/I").c_str()) )
    , ID( tree, CName(name, "ID").c_str(),  CLeaf(name, "ID", type_map::signed_short_type).c_str() )
    , veto( tree, CName(name, "Veto").c_str(), CLeaf(name, "Veto", type_map::bool_type).c_str() )
    , cfd_fail( tree, CName(name, "CFDfail").c_str(), CLeaf(name, "CFDfail", type_map::bool_type).c_str() )
    , energy( tree, CName(name, "Energy").c_str(), CLeaf(name, "Energy", type_map::double_type).c_str() )
    , cfd_corr( tree, CName(name, "CFDcorr").c_str(), CLeaf(name, "CFDcorr", type_map::double_type).c_str() )
    , timestamp(tree, CName(name, "Timestamp").c_str(), CLeaf(name, "Timestamp", type_map::signed_long_type).c_str() )
{}

void TreeData::push_back(const word_t &event)
{
    auto *detector = GetDetectorPtr(event.address);
    ID.push_back(( detector->type == clover ) ? detector->detectorNum*NUM_CLOVER_CRYSTALS + detector->telNum : detector->detectorNum );
    veto.push_back(event.veto);
    cfd_fail.push_back(event.cfdfail);
    energy.push_back(event.energy);
    cfd_corr.push_back(event.cfdcorr);
    timestamp.push_back(event.timestamp);
    ++entries;
}

TreeEvent::TreeEvent(TTree *tree)
    : clover(tree, "clover")
    , labrL(tree, "labrL")
    , labrS(tree, "labrS")
    , labrF(tree, "labrF")
    , ring(tree, "ring")
    , sect(tree, "sect")
    , back(tree, "back")
    , rf(tree, "rf"){}

TreeEvent &TreeEvent::operator=(const Event &event)
{
    // First we will clear everything
    clear();

    // Then we will go type by type and add events
    for ( auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, DetectorType::clover} ){

    }

    return *this;
}

void TreeEvent::push_back(const word_t &event)
{
    switch ( GetDetectorPtr(event.address)->type ) {
        case DetectorType::clover :
            clover.push_back(event);
            break;
        case DetectorType::labr_3x8 :
            labrL.push_back(event);
            break;
        case DetectorType::labr_2x2_ss :
            labrS.push_back(event);
            break;
        case DetectorType::labr_2x2_fs :
            labrF.push_back(event);
            break;
        case DetectorType::de_ring :
            ring.push_back(event);
            break;
        case DetectorType::de_sect :
            sect.push_back(event);
            break;
        case DetectorType::eDet :
            back.push_back(event);
            break;
        case DetectorType::rfchan :
            rf.push_back(event);
            break;
        default:
            break;
    }
}