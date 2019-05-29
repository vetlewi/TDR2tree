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

#include <stdint.h>
#include <vector>

#include "BasicStruct.h"

class TTree;
class TH2;


struct EventEntry
{
    int16_t ID;         //!< ID of the detector of this event.
    int16_t e_raw;      //!< Raw energy value.
    double energy;      //!< Energy of the event [keV].
    double tfine;       //!< Correction to the timestamp [ns].
    int64_t tcoarse;    //!< Timestamp [ns].
    explicit EventEntry() : ID( 0 ), energy( 0 ), tfine( 0 ), tcoarse( 0 ){}
    EventEntry(const int16_t &id, const double &e, const double &tf, const int64_t tc) : ID( id ), energy( e ), tfine( tf ), tcoarse( tc ){}
    explicit EventEntry(const word_t &word);
};

class Event
{

private:

    std::vector<EventEntry> ringEvent;      //!< Ring event structures.

    std::vector<EventEntry>  sectEvent;      //!< Sector event structures.

    std::vector<EventEntry> backEvent;      //!< Back event structures.

    std::vector<EventEntry> labrLEvent;     //!< Large volume LaBr event structures.

    std::vector<EventEntry> labrSEvent;     //!< Slow FTA LaBr event structures.

    std::vector<EventEntry> labrFEvent;     //!< Fast FTA LaBr event structures.

    std::vector<EventEntry> cloverEvent;    //!< Clover event structures.

    std::vector<EventEntry>  rfEvent;        //!< RF signal event structures.


public:

    //! Constructor.
    /*!
     * This constructor is meant to use for the TreeManager. This will set the branches of the tree.
     * \param tree ROOT TTree to attach branches to.
     */
    explicit Event(TTree *tree);

    //! Constructor.
    explicit Event(const std::vector<word_t> &event);

    //! Assignment operator.
    Event &operator=(const Event &event);

    void RunAddback(TH2 *ab_t_clover /*!< Matrix to fill in order to set propper time gates for clover addback */);

    //! Set all events.
    static std::vector<Event> BuildPGEvents(const std::vector<word_t> &raw_data, TH2 *ab_hist = nullptr, double coins_time = 3000.);

    //! Set all events.
    static std::vector<Event> BuildEvent(const std::vector<word_t> &raw_data, TH2 *ab_hist = nullptr, double gap_time=1500.);

    inline const std::vector<EventEntry> &GetRingEvent() const { return ringEvent; }

    inline const std::vector<EventEntry> &GetSectEvent() const { return sectEvent; }

    inline const std::vector<EventEntry> &GetBackEvent() const { return backEvent; }

    inline const std::vector<EventEntry> &GetLabrLEvent() const { return labrLEvent; }

    inline const std::vector<EventEntry> &GetLabrSEvent() const { return labrSEvent; }

    inline const std::vector<EventEntry> &GetLabrFEvent() const { return labrFEvent; }

    inline const std::vector<EventEntry> &GetCloverEvent() const { return cloverEvent; }

    inline const std::vector<EventEntry> &GetRFEvent() const { return rfEvent; }
};


#endif // EVENT_H
