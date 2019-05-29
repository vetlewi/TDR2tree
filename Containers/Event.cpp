#include <utility>

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
#include <iostream>

extern ProgressUI progress;


EventEntry::EventEntry(const word_t &word)
    : ID(  (GetDetector(word.address).type != clover) ? GetDetector(word.address).detectorNum : GetDetector(word.address).detectorNum*NUM_CLOVER_CRYSTALS + GetDetector(word.address).telNum )
    , e_raw( word.adcdata )
    , energy( CalibrateEnergy(word) )
    , tfine( word.cfdcorr )
    , tcoarse( word.timestamp )
{
}

bool EventData::Add(const word_t &w)
{
    if ( mult < MAX_NUM ){
        ID[mult] = (GetDetector(w.address).type != clover) ? GetDetector(w.address).detectorNum : GetDetector(w.address).detectorNum*NUM_CLOVER_CRYSTALS + GetDetector(w.address).telNum;
        e_raw[mult] = w.adcdata;
        energy[mult] = CalibrateEnergy(w);
        tfine[mult] = w.cfdcorr;
        tcoarse[mult++] = w.timestamp;
        return true;
    }
    return false;
}


bool EventData::Add(const uint16_t &id, const uint16_t &raw, const double &e, const double &fine, const int64_t &coarse)
{
    if ( mult < MAX_NUM ){
        ID[mult] = id;
        e_raw[mult] = raw;
        energy[mult] = e;
        tfine[mult] = fine;
        tcoarse[mult++] = coarse;
        return true;
    }
    return false;
}



void EventData::SetupBranch(TTree *tree, const char *baseName)
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

Event::Event(TTree *tree)
{
    ringData.SetupBranch(tree, "ring");
    sectData.SetupBranch(tree, "sect");
    backData.SetupBranch(tree, "back");
    labrLData.SetupBranch(tree, "labrL");
    labrSData.SetupBranch(tree, "labrS");
    labrFData.SetupBranch(tree, "labrF");
    cloverData.SetupBranch(tree, "clover");
    rfData.SetupBranch(tree, "rf");
    tree->BranchRef();
}

Event::Event(const std::vector<word_t> &event)
{
    DetectorInfo_t dinfo;
    for (const auto & it : event){
        dinfo = GetDetector(it.address);

        switch ( dinfo.type ){

            case de_ring : {
                if ( !ringData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many ring words." << std::endl;
                }
                break;
            }

            case de_sect : {
                if ( !sectData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many sect words." << std::endl;
                }
                break;
            }

            case eDet : {
                if ( !backData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many back words." << std::endl;
                }
                break;
            }

            case labr_3x8 : {
                if ( !labrLData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrL words." << std::endl;
                }
                break;
            }

            case labr_2x2_ss : {
                if ( !labrSData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrS words." << std::endl;
                }
                break;
            }

            case labr_2x2_fs : {
                if ( !labrFData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrF words." << std::endl;
                }
                break;
            }

            case clover : {
                if ( !cloverData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many clover words." << std::endl;
                }
                break;
            }

            case rfchan : {
                if ( !rfData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many rf words." << std::endl;
                }
                break;
            }
            default :
                break;
        }
    }
}


Event &Event::operator=(const Event &event)
{

    ringData = event.ringData;
    sectData = event.sectData;
    backData = event.backData;
    labrLData = event.labrLData;
    labrSData = event.labrSData;
    labrFData = event.labrFData;
    cloverData = event.cloverData;
    rfData = event.rfData;

    return *this;
}

Event &Event::operator=(const std::vector<word_t> &event)
{
    Reset();
    DetectorInfo_t dinfo;
    for (const auto & it : event){
        dinfo = GetDetector(it.address);
        switch ( dinfo.type ){

            case de_ring : {
                if ( !ringData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many ring words." << std::endl;
                }
                break;
            }

            case de_sect : {
                if ( !sectData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many sect words." << std::endl;
                }
                break;
            }

            case eDet : {
                if ( !backData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many back words." << std::endl;
                }
                break;
            }

            case labr_3x8 : {
                if ( !labrLData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrL words." << std::endl;
                }
                break;
            }

            case labr_2x2_ss : {
                if ( !labrSData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrS words." << std::endl;
                }
                break;
            }

            case labr_2x2_fs : {
                if ( !labrFData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many labrF words." << std::endl;
                }
                break;
            }

            case clover : {
                if ( !cloverData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many clover words." << std::endl;
                }
                break;
            }

            case rfchan : {
                if ( !rfData.Add(it) ){
                    std::cerr << __PRETTY_FUNCTION__ << ": Too many rf words." << std::endl;
                }
                break;
            }
            default :
                break;
        }
    }
    return *this;
}


void Event::RunAddback(TH2 *ab_t_clover)
{
    // We set up a vector for each clover.
    std::vector<EventEntry> cevent[NUM_CLOVER_DETECTORS];
    for (int i = 0 ; i < cloverData.mult ; ++i){
        cevent[cloverData.ID[i]/4].emplace_back(cloverData.ID[i], cloverData.e_raw[i], cloverData.energy[i], cloverData.tfine[i], cloverData.tcoarse[i]);

    }
    cloverData.Reset();
    std::vector<EventEntry> v, v_new;
    double e, tdiff;
    for (size_t n = 0 ; n < NUM_CLOVER_DETECTORS ; ++n){
        v = cevent[n];
        std::sort(v.begin(), v.end(), [](EventEntry lhs, EventEntry rhs){ return lhs.energy > rhs.energy; });
        while ( !v.empty() ){
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
            cloverData.Add(v[0].ID, 0, e, v[0].tfine, v[0].tcoarse);
        }
    }
}


std::vector<Event> Event::BuildPGEvents(const std::vector<word_t> &raw_data, TH2 *ab_hist, double coins_time)
{
    std::vector<Event> events;
    DetectorInfo_t trigger;
    double timediff;
    size_t i, j, start=0, stop=0;
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

void Event::BuildPGAndFill(const std::vector<word_t> &raw_data, HistManager *hm, TreeManager<Event> *tm, TH2 *ab_hist, double coins_time)
{
    DetectorInfo_t trigger;
    double timediff;
    size_t i, j, start=0, stop=0;
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
        Event evt(event);
        if ( ab_hist != nullptr )
            evt.RunAddback(ab_hist);
        if ( hm ) hm->AddEntry(evt);
        if ( tm ) tm->AddEntry(evt);
    }
}

void Event::BuildAndFill(const std::vector<word_t> &raw_data, HistManager *hm, TreeManager<Event> *tm, TH2 *ab_hist, double coins_time)
{
    double timediff;
    size_t i, j;
    progress.StartBuildingEvents(raw_data.size());
    for (i = 0 ; i < raw_data.size() ; ++i){
        std::vector<word_t> event;
        progress.UpdateEventBuildingProgress(i);
        for (j = i ; j < raw_data.size() - 1 ; ++j){
            timediff = abs(raw_data[j + 1].timestamp - raw_data[j].timestamp);
            event.push_back(raw_data[j]);
            if (timediff > coins_time) {
                i = j + 1;
                break;
            }
        }
        Event evt(event);
        if ( ab_hist != nullptr )
            evt.RunAddback(ab_hist);
        if ( hm ) hm->AddEntry(evt);
        if ( tm ) tm->AddEntry(evt);
    }
}

EventBuilder::EventBuilder(std::vector<word_t> data, TH2 *ab)
    : raw_data(std::move( data ))
    , current_pos( 0 )
    , ab_hist( ab )
{
    progress.StartBuildingEvents(raw_data.size());
}

bool EventBuilder::GetEvent(Event &evt)
{
    DetectorInfo_t trigger;
    double timediff;
    size_t i, j, start = current_pos, stop = current_pos+1;
    for (i = current_pos ; i < raw_data.size() ; ++i){
        trigger = GetDetector(raw_data[i].address);
        progress.UpdateEventBuildingProgress(i);
        if (trigger.type != eDet) // Skip to next word.
            continue;
        for (size_t j = i; j > 0; --j) {
            timediff = abs(raw_data[i].timestamp - raw_data[j - 1].timestamp);
            if (timediff > 1500) {
                start = j;
                break;
            }
        }

        for (j = i; j < raw_data.size() - 1; ++j) {
            timediff = abs(raw_data[i].timestamp - raw_data[j + 1].timestamp);
            if (timediff > 1500) {
                stop = j + 1;
                break;
            }
        }
        std::vector<word_t> ev;
        for (j = start ; j < stop ; ++j){
            ev.push_back(raw_data[j]);
        }
        evt = Event(ev);
        if ( ab_hist != nullptr)
            evt.RunAddback(ab_hist);
        return true;
    }
    return false;
}
