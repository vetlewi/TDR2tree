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
#include "Parameters.h"
#include "Histograms.h"
#include "Histogram2D.h"

#include <algorithm>
#include <list>

// TODO: Test addback method

// Function that takes a list of entries compare time
template<class T>
std::vector<T> Addback(std::vector<T> &tmp_evt, Histogram2Dp ab_t_clover, const int &dno)
{
    std::vector<T> res;

    // First sort by energy
    std::sort(tmp_evt.begin(), tmp_evt.end(),[](const T &lhs, const T &rhs){  return lhs.energy > rhs.energy; });
    std::list events(tmp_evt.begin(), tmp_evt.end());

    // Work our way through the list until it is empty!
    while ( !events.empty() ){
        res.push_back(events.front());
        events.pop_front();

        // Unfortunatly we also need to increment the add-back time spectra
        std::for_each(events.begin(), events.end(), [res, ab_t_clover, dno](const T &event){
            auto tdiff = double(event.timestamp - res.back().timestamp) + (event.cfdcorr - res.back().cfdcorr);
            ab_t_clover->Fill(tdiff, dno);
        });

        // Next we will search for the next element that are within the defined time window
        while ( true ){
            auto next = std::find_if(events.begin(), events.end(), [res](const T &event){
                auto tdiff = double(event.timestamp - res.back().timestamp) + (event.cfdcorr - res.back().cfdcorr);
                return CheckTimeGateAddback(tdiff); });
            if ( next == events.end() )
                break;

            // If we get here then that means that we need to do the addback
            res.back().energy += next->energy; // Add energy of the second!! Yay! Then we can remove it!!!
            events.erase(next); // And we can erase this event
        }
    }
    return res;
}

Event::Event(std::vector<word_t> event)
    : event_data(std::move( event ))
{
    // Next step is to sort the data we just got!
    std::sort(event_data.begin(), event_data.end(), [](const word_t &lhs, const word_t &rhs){
        return GetDetectorPtr(lhs.address)->type < GetDetectorPtr(rhs.address)->type;
    });

    for ( auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, clover} ){
        auto begin = std::find_if(event_data.begin(), event_data.end(), [&type](const word_t &next){
            return GetDetectorPtr(next.address)->type == type;
        });
        auto end = std::find_if(begin, event_data.end(), [&type](const word_t &next){
            return GetDetectorPtr(next.address)->type != type;
        });

        type_bounds[type]._begin = &begin[0];
        type_bounds[type]._end = &end[0];
    }
}

Event::Event(const std::vector<word_t>::iterator _begin, const std::vector<word_t>::iterator _end)
    : event_data(_begin, _end)
{
    // Next step is to sort the data we just got!
    std::sort(event_data.begin(), event_data.end(), [](const word_t &lhs, const word_t &rhs){
        return GetDetectorPtr(lhs.address)->type < GetDetectorPtr(rhs.address)->type;
    });

    auto begin = event_data.begin();
    for ( auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, clover} ){
        auto end = std::find_if(begin, event_data.end(), [&type](const word_t &next){
            return GetDetectorPtr(next.address)->type == type+1;
        });

        if ( end == event_data.end() ){
            type_bounds[type]._begin = &end[0];
            type_bounds[type]._end = &end[0];
            continue;
        }

        type_bounds[type]._begin = &begin[0];
        type_bounds[type]._end = &end[0];
        begin = end;
    }
}


void Event::RunAddback(Histogram2Dp ab_t_clover)
{
    // I want to update this to make it a little easier to understand what is going on.
    // First we will separate events by their detector

    // We set up a vector for each clover.
    std::vector<word_t> cevent[NUM_CLOVER_DETECTORS];
    const DetectorInfo_t *dinfo;
    for ( auto &evt : GetDetector(DetectorType::clover) ){
        dinfo = GetDetectorPtr(evt.address);
        cevent[dinfo->telNum].push_back(evt);
    }

    // Next we will iterate through all detectors
    int dno = -1;
    for ( auto &event : cevent ){
        ++dno;
        if ( event.empty() )
            continue;
        auto add_back_events = Addback(event, ab_t_clover, dno);
        std::for_each(add_back_events.begin(), add_back_events.end(),
                      [this](const auto &evt){ this->event_data.push_back(evt); });
    }
}