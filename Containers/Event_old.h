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

#include "BasicStruct.h"

class TTree;
class TH2;

#include "HistManager.h"
#include "TreeManager.h"

struct EventEntry
{
    uint16_t ID;         //!< ID of the detector of this event.
    uint16_t e_raw;      //!< Raw energy value.
    double energy;      //!< Energy of the event [keV].
    double tfine;       //!< Correction to the timestamp [ns].
    int64_t tcoarse;    //!< Timestamp [ns].
    explicit EventEntry() : ID( 0 ), e_raw( 0 ), energy( 0 ), tfine( 0 ), tcoarse( 0 ){}
    EventEntry(const uint16_t &id, const uint16_t &raw, const double &e, const double &tf, const int64_t &tc) : ID( id ), e_raw( raw ), energy( e ), tfine( tf ), tcoarse( tc ){}
    explicit EventEntry(const word_t &word);
};

#define MAX_NUM 128

struct EventData
{
    int mult;
    uint16_t ID[MAX_NUM];
    uint16_t e_raw[MAX_NUM];
    double energy[MAX_NUM];
    double tfine[MAX_NUM];
    int64_t tcoarse[MAX_NUM];

    bool Add(const EventEntry &e);
    bool Add(const word_t &w);
    bool Add(const uint16_t &id, const uint16_t &raw, const double &e, const double &fine, const int64_t &coarse);
    void Reset(){ mult = 0; }

    void SetupBranch(TTree *tree, const char *baseName, bool validated=false);

    EventData() : mult( 0 ), ID{ 0 }, e_raw{ 0 }, energy{ 0 }, tfine{ 0 }, tcoarse{ 0 }{}

    inline EventEntry operator[](const int &num) const
        { return ( num < MAX_NUM ) ? EventEntry(ID[num], e_raw[num], energy[num], tfine[num], tcoarse[num]) : EventEntry(); }
};

inline bool operator<(const int &lhs, const EventData &rhs){ return lhs < rhs.mult; }
inline bool operator>(const int &lhs, const EventData &rhs){ return lhs > rhs.mult; }
inline bool operator<(const EventData &lhs, const int &rhs){ return lhs.mult < rhs; }
inline bool operator>(const EventData &lhs, const int &rhs){ return lhs.mult > rhs; }

class Event
{

private:

    EventData ringData; //!< Ring event structure.
    EventData sectData; //!< Sector event structure.
    EventData backData;
    EventData labrLData;
    EventData labrSData;
    EventData labrFData;
    EventData cloverData;
    EventData rfData;

public:

    //! Zero constructor.
    Event() = default;

    //! Constructor.
    /*!
     * This constructor is meant to use for the TreeManager. This will set the branches of the tree.
     * \param tree ROOT TTree to attach branches to.
     */
    explicit Event(TTree *tree, bool validated=false);

    //! Constructor.
    explicit Event(const std::vector<word_t> &event);

    //! Assignment operator.
    Event &operator=(const Event &event);

    //! Assignment operator.
    Event &operator=(const std::vector<word_t> &event);

    //! Reset event.
    inline void Reset(){ ringData.Reset(); sectData.Reset(); backData.Reset(); labrLData.Reset(); labrSData.Reset(); labrFData.Reset(); cloverData.Reset(); rfData.Reset(); }

    void RunAddback(TH2 *ab_t_clover /*!< Matrix to fill in order to set propper time gates for clover addback */);

    //! Function to check if event is good. Also, only keep the sect/ring data that are acceptable.
    bool IsGood();

    //! Set all events.
    static std::vector<Event> BuildPGEvents(const std::vector<word_t> &raw_data, TH2 *ab_hist = nullptr, double coins_time = 3000.);

    //! Build and fill events.
    static void BuildPGAndFill(const std::vector<word_t> &raw_data, HistManager *hm, TreeManager<Event> *tm, TH2 *ab_hist = nullptr, double coins_time = 3000.);
    static void BuildAndFill(const std::vector<word_t> &raw_data, HistManager *hm, TreeManager<Event> *tm, TH2 *ab_hist = nullptr, double coins_time = 3000.);

    //! Set all events.
    static std::vector<Event> BuildEvent(const std::vector<word_t> &raw_data, TH2 *ab_hist = nullptr, double gap_time=1500.);

    inline const EventData &GetRingEvent() const { return ringData; }

    inline const EventData &GetSectEvent() const { return sectData; }

    inline const EventData &GetBackEvent() const { return backData; }

    inline const EventData &GetLabrLEvent() const { return labrLData; }

    inline const EventData &GetLabrSEvent() const { return labrSData; }

    inline const EventData &GetLabrFEvent() const { return labrFData; }

    inline const EventData &GetCloverEvent() const { return cloverData; }

    inline const EventData &GetRFEvent() const { return rfData; }
};

class EventBuilder {

private:
    std::vector<word_t> raw_data;
    size_t current_pos;
    TH2 *ab_hist;

public:
    explicit EventBuilder(std::vector<word_t> data, TH2 *ab = nullptr);// : raw_data( data ), current_pos( 0 ), ab_hist( ab ){}

    bool GetEvent(Event &evt);

};


#endif // EVENT_H
