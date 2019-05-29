/********************************************************************************
 * Copyright (C) 2019 Vetle W. Ingeberg                                         *
 * Author: Vetle Wegner Ingeberg, vetlewi@fys.uio.no                            *
 *                                                                              *
 * ---------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU General Public License as published by the        *
 * Free Software Foundation; either version 3 of the license, or (at your       *
 * option) any later version.                                                   *
 *                                                                              *
 * This program is distributed in the hope that it will be useful, but          *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General    *
 * Public License for more details.                                             *
 *                                                                              *
 * You should have received a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                     *
 *                                                                              *
 ********************************************************************************/

#include "Event.h"

#include "Calibration.h"
#include "experimentsetup.h"
#include "ProgressUI.h"

#include <exception>
#include <string>
#include <algorithm>


#include <TTree.h>
#include <TH2.h>

extern ProgressUI progress;


EventEntry::EventEntry(const word_t &word)
    : ID(  (GetDetector(word.address).type != clover) ? GetDetector(word.address).detectorNum : GetDetector(word.address).detectorNum*NUM_CLOVER_CRYSTALS + GetDetector(word.address).telNum )
    , energy( CalibrateEnergy(word) )
    , tfine( word.cfdcorr )
    , tcoarse( word.timestamp )
{
}

Event::Event(TTree *tree)
{
    tree->Branch("ringEvent", &ringEvent);
    tree->Branch("sectEvent", &sectEvent);
    tree->Branch("backEvent", &backEvent);
    tree->Branch("labrLEvent", &labrLEvent);
    tree->Branch("labrSEvent", &labrSEvent);
    tree->Branch("labrFEvent", &labrFEvent);
    tree->Branch("cloverEvent", &cloverEvent);
    tree->Branch("rfEvent", &rfEvent);
    tree->BranchRef();
}

Event::Event(const std::vector<word_t> &event)
{
    DetectorInfo_t dinfo;

    for (const auto & it : event){
        dinfo = GetDetector(it.address);

        switch ( dinfo.type ){

            case de_ring : {
                ringEvent.emplace_back(it);
                break;
            }

            case de_sect : {
                sectEvent.emplace_back(it);
                break;
            }

            case eDet : {
                backEvent.emplace_back(it);
                break;
            }

            case labr_3x8 : {
                labrLEvent.emplace_back(it);
                break;
            }

            case labr_2x2_ss : {
                labrSEvent.emplace_back(it);
                break;
            }

            case labr_2x2_fs : {
                labrFEvent.emplace_back(it);
                break;
            }

            case clover : {
                cloverEvent.emplace_back(it);
                break;
            }

            case rfchan : {
                rfEvent.emplace_back(it);
                break;
            }
            default :
                break;
        }
    }
}


Event &Event::operator=(const Event &event)
{
    ringEvent.clear();
    std::copy(event.ringEvent.begin(), event.ringEvent.end(), std::back_inserter(ringEvent));

    sectEvent.clear();
    std::copy(event.sectEvent.begin(), event.sectEvent.end(), std::back_inserter(sectEvent));

    backEvent.clear();
    std::copy(event.backEvent.begin(), event.backEvent.end(), std::back_inserter(backEvent));

    labrLEvent.clear();
    std::copy(event.labrLEvent.begin(), event.labrLEvent.end(), std::back_inserter(labrLEvent));

    labrSEvent.clear();
    std::copy(event.labrSEvent.begin(), event.labrSEvent.end(), std::back_inserter(labrSEvent));

    labrFEvent.clear();
    std::copy(event.labrFEvent.begin(), event.labrFEvent.end(), std::back_inserter(labrFEvent));

    cloverEvent.clear();
    std::copy(event.cloverEvent.begin(), event.cloverEvent.end(), std::back_inserter(cloverEvent));

    rfEvent.clear();
    std::copy(event.rfEvent.begin(), event.rfEvent.end(), std::back_inserter(rfEvent));

    return *this;
}


void Event::RunAddback(TH2 *ab_t_clover)
{
    // We set up a vector for each clover.
    std::vector<EventEntry> cevent[NUM_CLOVER_DETECTORS];
    for (const auto & it : cloverEvent){
        cevent[it.ID/4].push_back(it);

    }
    cloverEvent.clear();
    std::vector<EventEntry> v, v_new;
    double e, tdiff;
    for (size_t n = 0 ; n < NUM_CLOVER_DETECTORS ; ++n){
        v = cevent[n];
        std::sort(v.begin(), v.end(), [](EventEntry lhs, EventEntry rhs){ return lhs.energy > rhs.energy; });
        while ( v.size() > 0 ){
            v_new.clear();
            e = v[0].energy;
            for (size_t m = 1 ; m < v.size() ; ++m){
                tdiff = (v[m].tcoarse - v[0].tcoarse);
                tdiff += (v[m].tfine - v[0].tfine);
                ab_t_clover->Fill(tdiff, n);
                if ( CheckTimeGateAddback(tdiff) ){
                    e += v[m].energy;
                } else {
                    v_new.push_back(v[m]);
                }
            }
            v = v_new;
            cloverEvent.emplace_back(v[0].ID, e, v[0].tfine, v[0].tcoarse);
        }
    }
}


std::vector<Event> Event::BuildPGEvents(const std::vector<word_t> &raw_data, TH2 *ab_hist, double coins_time)
{
    std::vector<Event> events;
    DetectorInfo_t trigger;
    double timediff;
    size_t i, j, start, stop;
    progress.StartBuildingEvents(raw_data.size());
    for (i = 0 ; i < raw_data.size() ; ++i) {
        trigger = GetDetector(raw_data[i].address);
        progress.UpdateEventBuildingProgress(i);
        if (trigger.type != eDet) // Skip to next word.
            continue;

        for (size_t j = i; j > 0; --j) {
            timediff = abs(raw_data[i].timestamp - raw_data[j - 1].timestamp);
            if (timediff > coins_time/2.) {
                start = j;
                break;
            }
        }

        for (j = i; j < raw_data.size() - 1; ++j) {
            timediff = abs(raw_data[i].timestamp - raw_data[j + 1].timestamp);
            if (timediff > coins_time/2.) {
                stop = j + 1;
                break;
            }
        }
        std::vector<word_t> event;
        event.push_back(raw_data[i]);
        for (j = start ; j < stop ; ++j){
            if ( j == i )
                continue;
            event.push_back(raw_data[j]);
        }
        events.emplace_back(event);
        if ( ab_hist != nullptr )
            events.back().RunAddback(ab_hist);
    }
    return events;
}

std::vector<Event> Event::BuildEvent(const std::vector<word_t> &raw_data, TH2 *ab_hist, double gap_time)
{
    std::vector<Event> events;
    double timediff;
    size_t i, j;
    progress.StartBuildingEvents(raw_data.size());
    for (i = 0 ; i < raw_data.size() ; ++i){
        std::vector<word_t> event;
        progress.UpdateEventBuildingProgress(i);
        for (j = i ; j < raw_data.size() - 1 ; ++j){
            timediff = abs(raw_data[j + 1].timestamp - raw_data[j].timestamp);
            event.push_back(raw_data[j]);
            if (timediff > gap_time) {
                i = j + 1;
                break;
            }
        }
        events.emplace_back(event);
        if ( ab_hist != nullptr )
            events.back().RunAddback(ab_hist);
    }
    return events;
}
