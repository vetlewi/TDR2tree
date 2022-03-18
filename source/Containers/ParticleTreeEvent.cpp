//
// Created by Vetle Wegner Ingeberg on 14/03/2022.
//

#include "ParticleTreeEvent.h"
#include "Event.h"
#include "Calibration.h"
#include "experimentsetup.h"

struct pUsable_entries {
    size_t num;
    const word_t *words[8];

    void add(const word_t *word){ if ( num < 7 ) words[num++] = word; }
    pUsable_entries() : num( 0 ){}
};



struct ParticleEvent {
    struct pHit {
        word_t hit;
        double timediff;
    };
    pHit ringHit;
    pHit sectHit;
    pHit backHit;
};

bool pGetParticleEvent(const Event &buffer, ParticleEvent &particle)
{
    auto trigger = buffer.GetTrigger();

    // If there is more than one sector within -50 and 150 ns of the trigger we will return
    for ( auto &entry : buffer.GetDetector(de_sect) ){
        auto timediff = double(entry.timestamp_raw - trigger.timestamp_raw);
        timediff += entry.cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(entry) - CalibrateTime(trigger);

        // If the time difference are within the time window, we will skip.
        if ( timediff > -50 && timediff < 150 && entry.address != trigger.address )
            return false;
    }

    // Find the prompt rings.
    pUsable_entries ring_entries;
    for ( auto &entry : buffer.GetDetector(de_ring) ){
        auto timediff = double(entry.timestamp_raw - trigger.timestamp_raw);
        timediff += entry.cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(entry) - CalibrateTime(trigger);

        // If the time difference are within the time window, we will skip.
        if ( timediff > -150 && timediff < 55 )
            ring_entries.add(&entry);
    }

    // We will now inspect the entries we found. If there is more than two we will not proceed.
    if ( ring_entries.num > 2 ||ring_entries.num == 0 )
        return false;

    // Next we can inspect the back detector. First we will check for prompt entries
    word_t back_entry{};
    int num_back = 0;
    for ( auto &entry : buffer.GetDetector(eDet) ){
        auto timediff = double(entry.timestamp_raw - trigger.timestamp_raw);
        timediff += entry.cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(entry) - CalibrateTime(trigger);

        // If the time difference are within the time window, we will skip.
        if ( timediff > -50 && timediff < 150 ){
            back_entry = entry;
            ++num_back;
        }
    }

    // If not only one back event, we cannot be certain about the event.
    if ( num_back != 1 )
        return false;


    if ( ring_entries.num == 1 ){
        particle.ringHit.hit = *ring_entries.words[0];
        auto entry = ring_entries.words[0];
        particle.ringHit.timediff = double(entry->timestamp_raw - trigger.timestamp_raw);
        particle.ringHit.timediff += entry->cfdcorr - trigger.cfdcorr;
        particle.ringHit.timediff += CalibrateTime(*entry) - CalibrateTime(trigger);
    } else if ( ring_entries.num == 2 ){
        if ( abs(GetDetectorPtr(ring_entries.words[1]->address)->detectorNum -
                 GetDetectorPtr(ring_entries.words[0]->address)->detectorNum) != 1 ){
            // The energy is the sum of the two
            particle.ringHit.hit = ( ring_entries.words[0]->energy > ring_entries.words[1]->energy ) ? *ring_entries.words[0] : *ring_entries.words[1];
            particle.ringHit.hit.energy = ring_entries.words[1]->energy + ring_entries.words[0]->energy;

            // From our analysis, we have seen that the odd numbered rings above 16 is somewhat
            // strange. We will use the time of the even ring if there is two entries.
            auto as_trigger = ( (GetDetectorPtr(ring_entries.words[0]->address)->detectorNum & 1)==0 ) ? ring_entries.words[0] : ring_entries.words[1];

            auto timediff = double(as_trigger->timestamp_raw - trigger.timestamp_raw);
            timediff += as_trigger->cfdcorr - trigger.cfdcorr;
            timediff += CalibrateTime(*as_trigger) - CalibrateTime(trigger);
            particle.ringHit.timediff = timediff;
        }

    }

    // We now have our particle event!!
    particle.sectHit = {trigger, 0};

    auto timediff = double(back_entry.timestamp_raw - trigger.timestamp_raw);
    timediff += back_entry.cfdcorr - trigger.cfdcorr;
    timediff += CalibrateTime(back_entry) - CalibrateTime(trigger);
    particle.backHit = {back_entry, timediff};
    return true;
}


ParticleData::ParticleData(TTree *tree)
    : ring(tree, "ring")
    , ringTime(tree, CName("ring", "Time").c_str(), CLeaf("ring", "Time", type_map::double_type).c_str())
    , sect(tree, "sect")
    , sectTime(tree, CName("sect", "Time").c_str(), CLeaf("sect", "Time", type_map::double_type).c_str())
    , back(tree, "back")
    , backTime(tree, CName("back", "Time").c_str(), CLeaf("back", "Time", type_map::double_type).c_str()){}

void ParticleData::Set(const ParticleEvent &event_data)
{
    ring = event_data.ringHit.hit;
    ringTime = event_data.ringHit.timediff;
    sect = event_data.sectHit.hit;
    sectTime = event_data.sectHit.timediff;
    back = event_data.backHit.hit;
    backTime = event_data.backHit.timediff;
}

ParticleTreeData::ParticleTreeData(TTree *tree, const char *name)
        : entries( tree, CName(name, "Mult").c_str(), CLeaf(name, "Mult", type_map::signed_integer_type).c_str() )
        , ID( tree, CName(name, "ID").c_str(),  CLeafs(name, "ID", type_map::unsigned_short_type).c_str() )
        , veto( tree, CName(name, "Veto").c_str(), CLeafs(name, "Veto", type_map::bool_type).c_str() )
        , cfd_fail( tree, CName(name, "CFDfail").c_str(), CLeafs(name, "CFDfail", type_map::bool_type).c_str() )
        , energy( tree, CName(name, "Energy").c_str(), CLeafs(name, "Energy", type_map::double_type).c_str() )
        , time(tree, CName(name, "Time").c_str(), CLeafs(name, "Time", type_map::double_type).c_str() )
        , timestamp(tree, CName(name, "Timestamp").c_str(), CLeafs(name, "Timestamp", type_map::signed_long_type).c_str() )
        , cfdcorr(tree, CName(name, "CFDcorr").c_str(), CLeafs(name, "CFDcorr", type_map::double_type).c_str() )
{}

void ParticleTreeData::push_back(const word_t &event, const word_t &trigger)
{
    auto *detector = GetDetectorPtr(event.address);
    ID.push_back(( detector->type == clover ) ? detector->detectorNum*NUM_CLOVER_CRYSTALS + detector->telNum : detector->detectorNum, entries );
    veto.push_back(event.veto, entries);
    cfd_fail.push_back(event.cfdfail, entries);
    energy.push_back(event.energy, entries);
    double tdiff = double(event.timestamp_raw - trigger.timestamp_raw);
    tdiff += event.cfdcorr - trigger.cfdcorr;
    tdiff += CalibrateTime(event) - CalibrateTime(trigger);

    time.push_back(tdiff, entries);
    timestamp.push_back(event.timestamp_raw, entries);
    cfdcorr.push_back(event.cfdcorr+CalibrateTime(event), entries);
    ++entries;
}

ParticleTreeEvent::ParticleTreeEvent(TTree *tree)
    : trigger(tree, "trigger")
    , clover(tree, "clover")
    , labrL(tree, "labrL")
    , labrS(tree, "labrS")
    , labrF(tree, "labrF")
    , rf(tree, "rf")
    , particle(tree)
    , have_valid_particle( false )
{}

ParticleTreeData &ParticleTreeEvent::GetData(const DetectorType &type)
{
    switch ( type ) {
        case DetectorType::clover : return clover;
        case labr_3x8 : return labrL;
        case labr_2x2_ss : return labrS;
        case labr_2x2_fs : return labrF;
        case rfchan : return rf;
        default: return clover;
    }
}

void ParticleTreeEvent::push_back(const word_t &event, const word_t &_trigger)
{
    switch ( GetDetectorPtr(event.address)->type ) {
        case DetectorType::clover :
            clover.push_back(event, _trigger);
            break;
        case DetectorType::labr_3x8 :
            labrL.push_back(event, _trigger);
            break;
        case DetectorType::labr_2x2_ss :
            labrS.push_back(event, _trigger);
            break;
        case DetectorType::labr_2x2_fs :
            labrF.push_back(event, _trigger);
            break;
        case DetectorType::rfchan :
            rf.push_back(event, _trigger);
        default:
            break;
    }
}

ParticleTreeEvent &ParticleTreeEvent::operator=(const Event &event)
{
    clear();

    word_t trigger_data = event.GetTrigger();
    trigger = trigger_data;
    // Set gamma detectors
    for ( auto &type : {DetectorType::clover, labr_3x8, labr_2x2_ss, labr_2x2_fs} ){
        auto data = event.GetDetector(type);
        GetData(type).add(data.begin(), data.end(), trigger_data);
    }
    ParticleEvent pevt{};
    have_valid_particle = pGetParticleEvent(event, pevt);
    if ( have_valid_particle )
        particle.Set(pevt);

    return *this;
}