//
// Created by Vetle Wegner Ingeberg on 15/04/2021.
//


#include "TreeEvent.h"

#include "experimentsetup.h"
#include "BasicStruct.h"
#include "Event.h"

std::string CName(const char *base_name, const char *identifier)
{
    return std::string(base_name) + std::string(identifier);
}

std::string CLeaf(const char *base_name, const char *identifier, const char type)
{
    return CName(base_name, identifier) + "/" + std::string(1, type);
}

std::string CLeafs(const char *base_name, const char *identifier, const char type)
{
    return CName(base_name, identifier) + "[" + CName(base_name, "Mult") + "]/" + std::string(1, type);
}

TreeData::TreeData(TTree *tree, const char *name)
    : entries( tree, CName(name, "Mult").c_str(), CLeaf(name, "Mult", type_map::signed_integer_type).c_str() )
    , ID( tree, CName(name, "ID").c_str(),  CLeaf(name, "ID", type_map::unsigned_short_type).c_str() )
    , veto( tree, CName(name, "Veto").c_str(), CLeaf(name, "Veto", type_map::bool_type).c_str() )
    , cfd_fail( tree, CName(name, "CFDfail").c_str(), CLeaf(name, "CFDfail", type_map::bool_type).c_str() )
    , energy( tree, CName(name, "Energy").c_str(), CLeaf(name, "Energy", type_map::double_type).c_str() )
    , cfd_corr( tree, CName(name, "CFDcorr").c_str(), CLeaf(name, "CFDcorr", type_map::double_type).c_str() )
    , timestamp(tree, CName(name, "Timestamp").c_str(), CLeaf(name, "Timestamp", type_map::signed_long_type).c_str() )
{}

void TreeData::push_back(const word_t &event)
{
    auto *detector = GetDetectorPtr(event.address);
    ID.push_back(( detector->type == clover ) ? detector->detectorNum*NUM_CLOVER_CRYSTALS + detector->telNum : detector->detectorNum, entries );
    veto.push_back(event.veto, entries);
    cfd_fail.push_back(event.cfdfail, entries);
    energy.push_back(event.energy, entries);
    cfd_corr.push_back(event.cfdcorr, entries);
    timestamp.push_back(event.timestamp, entries);
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

TreeData &TreeEvent::GetData(const DetectorType &type)
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
        default: return rf;
    }
}

TreeEvent &TreeEvent::operator=(Event &event)
{
    // First we will clear everything
    clear();

    // Then we will go type by type and add events
    for ( auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, DetectorType::clover} ){
        auto data = event.GetDetector(type);
        GetData(type).add(data.begin(), data.end());
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