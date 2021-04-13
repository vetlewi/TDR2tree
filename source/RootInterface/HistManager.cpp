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

#include "BasicStruct.h"

#include <cstdio>

#include <TH1.h>
#include <TH2.h>
#include <iostream>

#include "Histogram1D.h"
#include "Histogram2D.h"

extern ProgressUI progress;

HistManager::Detector_Histograms_t::Detector_Histograms_t(RootFileManager *fm, const std::string &name, const size_t &num)
    : time( fm->Mat(std::string("time_"+name), std::string("Time spectra "+name), 3000, -1500, 1500, "Time [ns]", num, 0, num, std::string(name+" ID").c_str()) )
    , energy( fm->Mat(std::string("energy_"+name), std::string("Energy spectra "+name), 65536, 0, 65536, "Energy [ch]", num, 0, num, std::string(name+" ID").c_str()) )
    , energy_cal( fm->Mat(std::string("energy_cal_"+name), std::string("energy spectra "+name+" (cal)"), 16384, 0, 16384, "Energy [keV]", num, 0, num, std::string(name+" ID").c_str()) )
    , mult( fm->Spec(std::string("mult_"+name), std::string("Multiplicity " + name), 128, 0, 128, "Multiplicity") )
{}

void HistManager::Detector_Histograms_t::Fill(const word_t &word)
{
    auto dinfo = GetDetector(word.address);
    auto dno = ( dinfo.type == DetectorType::clover ) ? dinfo.detectorNum * NUM_CLOVER_CRYSTALS +  dinfo.telNum : dinfo.detectorNum;
    energy->Fill(word.adcdata, dno);
    energy_cal->Fill(CalibrateEnergy(word), dno);
}

void HistManager::Detector_Histograms_t::Fill(const EventData &events, const EventEntry &start)
{
    mult->Fill(events.mult);
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
    mult->Fill(events.mult);
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
    , time_energy_sect_back( fm->Mat("time_energy_sect_back", "Energy vs. sector/back time", 1000, 0, 30000, "E energy [keV]", 3000, -1500, 1500, "t_{back} - t_{sector} [ns]") )
    , time_energy_ring_sect( fm->Mat("time_energy_ring_sect", "Energy vs. sector/back time", 1000, 0, 30000, "Sector energy [keV]", 3000, -1500, 1500, "t_{ring} - t_{sector} [ns]") )
{
}

void HistManager::AddEntry(const Event &buffer)
{
   // We will split each event such that only the entries that are closest to the RF event are counted
   // No we will use labrF 0 as reference
   /*int n_labrF0 = 0;
   EventEntry rfEvent;
   for ( int n = 0 ; n < buffer.GetLabrFEvent() ; ++n ){
       if ( buffer.GetLabrFEvent().ID[n] == 0 ) {
           ++n_labrF0;
           rfEvent = buffer.GetLabrFEvent()[n];
       }
   }*/


   if ( buffer.GetRFEvent().mult == 1 ){
       auto rfEvent = buffer.GetRFEvent()[0];

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

void HistManager::AddEntry(const word_t &word)
{
    auto *spec = GetSpec(GetDetector(word.address).type);
    if ( spec )
        spec->Fill(word);
}


void HistManager::AddEntries(const std::vector<Event> &evts)
{
    progress.StartFillingHistograms(evts.size());
    for (size_t i = 0 ; i < evts.size() ; ++i){
        AddEntry(evts[i]);
        progress.UpdateHistFillProgress(i);
    }
}

HistManager::Detector_Histograms_t *HistManager::GetSpec(const DetectorType &type)
{
    switch (type) {
        case DetectorType::labr_3x8 : return &labrL;
        case DetectorType::labr_2x2_ss : return &labrS;
        case DetectorType::labr_2x2_fs : return &labrF;
        case DetectorType::clover : return &clover;
        case DetectorType::de_ring : return &ring;
        case DetectorType::de_sect : return &sect;
        case DetectorType::eDet : return &back;
        default: return nullptr;
    }
}

void HistManager::AddEntries(std::vector<word_t> &evts)
{
    for ( int type = DetectorType::invalid ;
          type < DetectorType::unused ; ++type ){

        auto *hist = GetSpec(DetectorType(type));
        if ( !hist )
            continue;

        auto end = std::partition(evts.begin(), evts.end(), [&type](const word_t &word){
            return GetDetector(word.address).type == type;
        });

        std::for_each(evts.begin(), end, [&hist](const word_t &word){  hist->Fill(word); });
    }
}