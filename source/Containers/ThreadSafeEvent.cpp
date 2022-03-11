//
// Created by Vetle Wegner Ingeberg on 11/03/2022.
//

#include "ThreadSafeEvent.h"

#include <list>

#include <experimentsetup.h>
#include <Calibration.h>


template<class T>
std::vector<T> Addback(std::list<T> &events, ThreadSafeHistogram2D *ab_t_clover, const int &dno)
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

std::vector<word_t> RunAddback(std::vector<word_t> events, ThreadSafeHistogram2D *ab_t_clover)
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

ThreadSafeEvent::ThreadSafeEvent(std::vector<word_t> event, ThreadSafeHistogram2D *ab_t_clover)
        : trigger( {0, 0, 0, false, false, 0, 0, 0, 0} )
        , event_data(std::move( event ))
{
    index(ab_t_clover);
}

ThreadSafeEvent::ThreadSafeEvent(word_t trigger, std::vector<word_t> event, ThreadSafeHistogram2D *ab_t_clover)
        : trigger( trigger )
        , event_data(std::move( event ))
{
    index(ab_t_clover);
}

void ThreadSafeEvent::index(ThreadSafeHistogram2D *ab_t_clover)
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