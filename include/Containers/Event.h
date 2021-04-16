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

#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <vector>
#include <utility>

#include "BasicStruct.h"
#include "experimentsetup.h"
#include "Histograms.h"

#include "HistManager.h"
#include "TreeManager.h"
#include "ProgressUI.h"

class TreeEvent;

struct Subevent {
    word_t *_begin;
    word_t *_end;

    inline word_t *begin(){ return _begin; }
    inline word_t *end(){ return _end; }

    inline word_t *begin() const { return _begin; }
    inline word_t *end() const { return _end; }

    inline size_t size() const { return _end - _begin; }
};

class Event
{
public:
    friend class TreeEvent;

private:

    std::vector<word_t> event_data;
    std::map<DetectorType, Subevent> type_bounds;

    void index(Histogram2Dp ab_t_clover);

public:

    //! Constructor.
    explicit Event(std::vector<word_t> event, Histogram2Dp ab_t_clover = nullptr);

    template<class It>
    Event(It begin, It end, Histogram2Dp ab_t_clover = nullptr) : event_data(begin, end){ index(ab_t_clover); }

    //void RunAddback(Histogram2Dp ab_t_clover /*!< Matrix to fill in order to set propper time gates for clover addback */);

    //! Build and fill events.
    template<class It, class TE>
    static void BuildPGAndFill(It start, It stop, HistManager *hm, TreeManager<TE> *tm, Histogram2Dp ab_hist = nullptr, const DetectorType &trigger = eDet, double coins_time = 3000., ProgressUI *prog = nullptr);

    template<class It, class TE>
    static void BuildAndFill(It start, It stop, HistManager *hm, TreeManager<TE> *tm, Histogram2Dp ab_hist = nullptr, double coins_time = 3000., ProgressUI *prog = nullptr);

    inline Subevent &GetDetector(const DetectorType &type){ return type_bounds.at(type); }

};

template<class It, class TE>
void Event::BuildPGAndFill(It _start, It _stop, HistManager *hm, TreeManager<TE> *tm, Histogram2Dp ab_hist,
                           const DetectorType &trigger, double coins_time, ProgressUI *prog)
{

    auto begin = _start;
    auto end = _stop;
    auto it = _start;

    size_t count = 0;
    if ( prog )
        progress.StartFillingTree(_stop-_start);

    while ( it < end ){

        if ( GetDetectorPtr(it->address)->type != trigger ) {
            ++it;
            continue;
        }

        auto start = it;
        while ( start >= begin ){
            if ( abs(double(it->timestamp - start->timestamp) + (it->cfdcorr - start->cfdcorr)) > coins_time )
                break;
            --start;
        }
        start = ( start < begin ) ? begin : start;

        auto stop = it + 1;
        while ( stop < end ){
            if ( abs(double(it->timestamp - stop->timestamp) + (it->cfdcorr - stop->cfdcorr)) > coins_time )
                break;
            ++stop;
        }

        Event evt(std::vector<word_t>(start, stop), ab_hist);
        //if ( ab_hist ) evt.RunAddback(ab_hist);
        if ( hm ) hm->AddEntry(evt);
        if ( tm ) tm->AddEntry(evt);

        it = stop;

        if ( prog )
            prog->UpdateTreeFillProgress(count++);
    }

}

template<class It, class TE>
void Event::BuildAndFill(It _start, It _stop, HistManager *hm, TreeManager<TE> *tm, Histogram2Dp ab_hist, double coins_time, ProgressUI *prog)
{
    auto begin = _start;
    auto it = _start;
    auto end = _stop;

    if ( prog )
        progress.StartFillingTree(_stop-_start);

    while ( it < end - 1 ){
        if ( abs( double((it+1)->timestamp - it->timestamp) + ((it+1)->cfdcorr - it->cfdcorr) ) > coins_time ){
            Event evt(std::vector<word_t>(begin, it+1), ab_hist);
            //if ( ab_hist ) evt.RunAddback(ab_hist);
            if ( hm ) hm->AddEntry(evt);
            if ( tm ) tm->AddEntry(evt);
            begin = it+1;
        }
        ++it;

        if ( prog )
            prog->UpdateTreeFillProgress(it - _start);
    }
}

#endif // EVENT_H
