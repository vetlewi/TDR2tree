/*******************************************************************************
 * Copyright (C) 2019 Vetle W. Ingeberg                                        *
 * Author: Vetle Wegner Ingeberg, vetlewi@fys.uio.no                           *
 *                                                                             *
 * --------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the       *
 * Free Software Foundation; either version 3 of the license, or (at your      *
 * option) any later version.                                                  *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but         *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General   *
 * Public License for more details.                                            *
 *                                                                             *
 * You should have recived a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                    *
 *                                                                             *
 *******************************************************************************/

#include "HistManager.h"
#include "Calibration.h"
#include "experimentsetup.h"
#include "Event.h"
#include "ProgressUI.h"

#include <cstdio>

#include <TH1.h>
#include <TH2.h>

extern ProgressUI progress;

HistManager::Detector_Histograms_t::Detector_Histograms_t(RootFileManager *fm, const std::string &name, const size_t &num)
    : time( fm->CreateTH2(std::string("time_"+name).c_str(), std::string("Time spectra "+name).c_str(), 3000, -1500, 1500, "Time [ns]", num, 0, num, std::string(name+" ID").c_str()) )
    , energy( fm->CreateTH2(std::string("energy_"+name).c_str(), std::string("Energy spectra "+name).c_str(), 16384, 0, 16374, "Energy [ch]", num, 0, num, std::string(name+" ID").c_str()) )
    , energy_cal( fm->CreateTH2(std::string("energy_cal_"+name).c_str(), std::string("energy spectra "+name+" (cal)").c_str(), 16384, 0, 16374, "Energy [keV]", num, 0, num, std::string(name+" ID").c_str()) )
{}

void HistManager::Detector_Histograms_t::Fill(const EventData &events, const EventEntry &start)
{
    for ( size_t n = 0 ; n < events.mult ; ++n ){
        if ( !events.cfd_fail[n] )
            time->Fill(double(events.tcoarse[n] - start.tcoarse) + (events.tfine[n] - start.tfine),
                        events.ID[n]);
        energy->Fill(events.e_raw[n], events.ID[n]);
        energy_cal->Fill(events.energy[n], events.ID[n]);
    }
}

void HistManager::Detector_Histograms_t::Fill(const EventData &events)
{
    for ( size_t n = 0 ; n < events.mult ; ++n ){
        energy->Fill(events.e_raw[n], events.ID[n]);
        energy_cal->Fill(events.energy[n], events.ID[n]);
    }
}

HistManager::HistManager(RootFileManager *fm)
    : ring( fm, "ring", NUM_SI_RING )
    , sect( fm, "sect", NUM_SI_SECT )
    , back( fm, "back", NUM_SI_BACK )
    , labrL( fm, "labrL", NUM_LABR_3X8_DETECTORS )
    , labrS( fm, "labrS", NUM_LABR_2X2_DETECTORS )
    , labrF( fm, "labrF", NUM_LABR_2X2_DETECTORS )
    , clover( fm, "clover", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS )
    , time_energy_sect_back( fm->CreateTH2("time_energy_sect_back", "Energy vs. sector/back time", 1000, 0, 30000, "E energy [keV]", 3000, -1500, 1500, "t_{back} - t_{sector} [ns]") )
    , time_energy_ring_sect( fm->CreateTH2("time_energy_ring_sect", "Energy vs. sector/back time", 1000, 0, 30000, "Sector energy [keV]", 3000, -1500, 1500, "t_{ring} - t_{sector} [ns]") )
{
}

void HistManager::AddEntry(const Event &buffer)
{
   // We will split each event such that only the entries that are closest to the RF event are counted
   // No we will use labrF 0 as reference
   int n_labrF0 = 0;
   EventEntry rfEvent;
   for ( int n = 0 ; n < buffer.GetLabrFEvent() ; ++n ){
       if ( buffer.GetLabrFEvent().ID[n] == 0 ) {
           ++n_labrF0;
           rfEvent = buffer.GetLabrFEvent()[n];
       }
   }


   if ( n_labrF0 == 1 ){

       ring.Fill(buffer.GetRingEvent(), rfEvent);
       sect.Fill(buffer.GetSectEvent(), rfEvent);
       back.Fill(buffer.GetBackEvent(), rfEvent);

       labrL.Fill(buffer.GetLabrLEvent(), rfEvent);
       labrS.Fill(buffer.GetLabrSEvent(), rfEvent);
       labrF.Fill(buffer.GetLabrFEvent(), rfEvent);
       clover.Fill(buffer.GetCloverEvent(), rfEvent);

   } else {
       ring.Fill(buffer.GetRingEvent());
       sect.Fill(buffer.GetSectEvent());
       back.Fill(buffer.GetBackEvent());

       labrL.Fill(buffer.GetLabrLEvent());
       labrS.Fill(buffer.GetLabrSEvent());
       labrF.Fill(buffer.GetLabrFEvent());
       clover.Fill(buffer.GetCloverEvent());
   }


   double timediff;
   EventEntry evt, rfEvt;
    for (int i = 0 ; i < buffer.GetSectEvent() ; ++i){
        rfEvt = buffer.GetSectEvent()[i];
        for (int j = 0 ; j < buffer.GetBackEvent() ; ++j){
            evt = buffer.GetBackEvent()[j];
            timediff = double(evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_energy_sect_back->Fill(evt.energy, timediff);
        }
        for (int j = 0 ; j < buffer.GetRingEvent() ; ++j) {
            evt = buffer.GetRingEvent()[j];
            if ( abs(rfEvt.energy - evt.energy) > 500 )
                continue;
            timediff = double(evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_energy_ring_sect->Fill(evt.energy, timediff);
        }
    }
}

void HistManager::AddEntries(const std::vector<Event> &evts)
{
    progress.StartFillingHistograms(evts.size());
    for (size_t i = 0 ; i < evts.size() ; ++i){
        AddEntry(evts[i]);
        progress.UpdateHistFillProgress(i);
    }
}
