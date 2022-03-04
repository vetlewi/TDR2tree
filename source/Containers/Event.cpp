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
std::vector<T> Addback(std::list<T> &events, Histogram2Dp ab_t_clover, const int &dno)
{
    std::vector<T> res;
    res.reserve(events.size());

    events.sort([](const T &lhs, const T &rhs){ return lhs.energy > rhs.energy; });

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
            res.back().veto = res.back().veto || next->veto;
            events.erase(next); // And we can erase this event
        }
    }
    return res;
}

std::vector<word_t> RunAddback(std::vector<word_t> events, Histogram2Dp ab_t_clover)
{
    std::vector<word_t> res;
    res.reserve(events.size());

    std::sort(events.begin(), events.end(), [](const word_t &lhs, const word_t &rhs){
        return GetDetectorPtr(lhs.address)->detectorNum < GetDetectorPtr(lhs.address)->detectorNum;
    });

    // Loop over all detectors
    std::list<word_t> cevents[NUM_CLOVER_DETECTORS];
    for ( auto &event : events ){
        cevents[GetDetectorPtr(event.address)->detectorNum].push_back(event);
    }
    int dno = -1;
    for ( auto &evts : cevents ){
        ++dno;
        if ( evts.empty() )
            continue;
        if ( evts.size() == 1 ) {
            res.push_back(evts.front());
            continue;
        }
        auto ab_events = Addback(evts, ab_t_clover, dno);
        res.insert(res.end(), ab_events.begin(), ab_events.end());
    }
    return res;
}

Event::Event(std::vector<word_t> event, Histogram2Dp ab_t_clover)
    : trigger( {0, 0, 0, false, false, 0, 0, 0, 0} )
    , event_data(std::move( event ))
{
    index(ab_t_clover);
}

Event::Event(word_t trigger, std::vector<word_t> event, Histogram2Dp ab_t_clover)
    : trigger( trigger )
    , event_data(std::move( event ))
{
    index(ab_t_clover);
}

void Event::index(Histogram2Dp ab_t_clover)
{
    // Next step is to sort the data we just got!
    std::sort(event_data.begin(), event_data.end(), [](const word_t &lhs, const word_t &rhs){
        return GetDetectorPtr(lhs.address)->type < GetDetectorPtr(rhs.address)->type;
    });

    // First we will perform the addback for the clover detectors
    if ( ab_t_clover ){
        // Now we will get the pointers to the first and the last clover elements
        auto begin = std::find_if(event_data.begin(), event_data.end(), [](const word_t &next){
            return GetDetectorPtr(next.address)->type == DetectorType::clover;
        });
        auto end = std::find_if_not(begin, event_data.end(), [](const word_t &next){
            return GetDetectorPtr(next.address)->type == DetectorType::clover;
        });
        if ( end - begin > 1 ) {
            auto ab_events = RunAddback(std::vector(begin, end), ab_t_clover);
            event_data.erase(begin, end);
            event_data.insert(event_data.end(), ab_events.begin(), ab_events.end());
        }
    }

    // Ensure that the data are indeed sorted. After this call we will never modify the data again!
    std::sort(event_data.begin(), event_data.end(), [](const word_t &lhs, const word_t &rhs){
        return GetDetectorPtr(lhs.address)->type < GetDetectorPtr(rhs.address)->type;
    });

    for ( auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, clover} ){
        auto begin = std::find_if(event_data.begin(), event_data.end(), [&type](const word_t &next){
            return GetDetectorPtr(next.address)->type == type;
        });
        auto end = std::find_if_not(begin, event_data.end(), [&type](const word_t &next){
            return GetDetectorPtr(next.address)->type == type;
        });

        type_bounds[type]._begin = &begin[0];
        type_bounds[type]._end = &end[0];
    }
}


/*void Event::RunAddback(Histogram2Dp ab_t_clover)
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

    // Erase unused entries
    event_data.erase(type_bounds[DetectorType::clover].begin(), type_bounds[DetectorType::clover].end());

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

    // Lastly we need to update our index
    type_bounds[DetectorType::clover]._begin = &std::find_if(event_data.begin(), event_data.end(),
         [](const word_t &word){ return GetDetectorPtr(word.address)->type == DetectorType::clover; })[0];
    type_bounds[DetectorType::clover]._begin = &std::find_if(event_data.begin(), event_data.end(),
         [](const word_t &word){ return GetDetectorPtr(word.address)->type == DetectorType::clover+1; })[0];
}*/