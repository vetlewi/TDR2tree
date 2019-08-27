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

HistManager::HistManager(RootFileManager *fm)
    : time_ring( fm->CreateTH2("time_ring", "Time spectra rings", 3000, -1500, 1500, "Time [ns]", NUM_SI_RING, 0, NUM_SI_RING, "Ring ID") )
    , time_sect( fm->CreateTH2("time_sect", "Time spectra sectors", 3000, -1500, 1500, "Time [ns]", NUM_SI_SECT, 0, NUM_SI_SECT, "Sector ID") )
    , time_back( fm->CreateTH2("time_back", "Time spectra back detector", 3000, -1500, 1500, "Time [ns]", NUM_SI_BACK, 0, NUM_SI_BACK, "Back ID") )
    , time_labrL( fm->CreateTH2("time_labrL", "Time spectra LaBr L", 30000, -1500, 1500, "Time [ns]", NUM_LABR_3X8_DETECTORS, 0, NUM_LABR_3X8_DETECTORS, "LaBr L ID") )
    , time_labrS( fm->CreateTH2("time_labrS", "Time spectra LaBr S", 3000, -1500, 1500, "Time [ns]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr S ID") )
    , time_labrF( fm->CreateTH2("time_labrF", "Time spectra LaBr F", 30000, -1500, 1500, "Time [ns]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr F ID") )
    , time_clover( fm->CreateTH2("time_clover", "Time spectra CLOVER", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0, NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, "CLOVER ID") )
    , time_energy_sect_back( fm->CreateTH2("time_energy_sect_back", "Energy vs. sector/back time", 1000, 0, 30000, "E energy [keV]", 3000, -1500, 1500, "t_{back} - t_{sector} [ns]") )
    , time_energy_ring_sect( fm->CreateTH2("time_energy_ring_sect", "Energy vs. sector/back time", 1000, 0, 30000, "Sector energy [keV]", 3000, -1500, 1500, "t_{ring} - t_{sector} [ns]") )
    , energy_ring( fm->CreateTH2("energy_ring", "Energy spectra rings", 16384, 0, 16384, "Energy [ch]", NUM_SI_RING, 0, NUM_SI_RING, "Ring ID") )
    , energy_sect( fm->CreateTH2("energy_sect", "Energy spectra sectors", 16384, 0, 16384, "Energy [ch]", NUM_SI_SECT, 0, NUM_SI_SECT, "Sector ID") )
    , energy_back( fm->CreateTH2("energy_back", "Energy spectra back detectors", 16384, 0, 16384, "Energy [ch]", NUM_SI_BACK, 0, NUM_SI_BACK, "Back ID") )
    , energy_labrL( fm->CreateTH2("energy_labrL", "Energy spectra LaBr L", 16384, 0, 16384, "Energy [ch]", NUM_LABR_3X8_DETECTORS, 0, NUM_LABR_3X8_DETECTORS, "LaBr L ID") )
    , energy_labrS( fm->CreateTH2("energy_labrS", "Energy spectra LaBr S", 16384, 0, 16384, "Energy [ch]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr S ID") )
    , energy_labrF( fm->CreateTH2("energy_labrF", "Energy spectra LaBr F", 16384, 0, 16384, "Energy [ch]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr F ID") )
    , energy_clover( fm->CreateTH2("energy_clover", "Energy spectra CLOVER", 16384, 0, 16384, "Energy [ch]", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0, NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, "CLOVER ID") )
    , energy_cal_ring( fm->CreateTH2("energy_cal_ring", "Energy spectra rings", 16384, 0, 16384, "Energy [keV]", NUM_SI_RING, 0, NUM_SI_RING, "Ring ID") )
    , energy_cal_sect( fm->CreateTH2("energy_cal_sect", "Energy spectra sectors", 16384, 0, 16384, "Energy [keV]", NUM_SI_SECT, 0, NUM_SI_SECT, "Sector ID") )
    , energy_cal_back( fm->CreateTH2("energy_cal_back", "Energy spectra back detectors", 16384, 0, 16384, "Energy [keV]", NUM_SI_BACK, 0, NUM_SI_BACK, "Back ID") )
    , energy_cal_labrL( fm->CreateTH2("energy_cal_labrL", "Energy spectra LaBr L", 16384, 0, 16384, "Energy [keV]", NUM_LABR_3X8_DETECTORS, 0, NUM_LABR_3X8_DETECTORS, "LaBr L ID") )
    , energy_cal_labrS( fm->CreateTH2("energy_cal_labrS", "Energy spectra LaBr S", 16384, 0, 16384, "Energy [keV]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr S ID") )
    , energy_cal_labrF( fm->CreateTH2("energy_cal_labrF", "Energy spectra LaBr F", 16384, 0, 16384, "Energy [keV]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr F ID") )
    , energy_cal_clover( fm->CreateTH2("energy_cal_clover", "Energy spectra CLOVER", 16384, 0, 16384, "Energy [keV]", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0, NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, "CLOVER ID") )
{

}

void HistManager::AddEntry(const Event &buffer)
{
    // First time spectra. We use the RF as reference.
    double timediff;
    int i, j;
    EventEntry rfEvt, evt;
    for (i = 0 ; i < buffer.GetRFEvent() ; ++i){
        rfEvt = buffer.GetRFEvent()[i];

        for (j = 0 ; j < buffer.GetRingEvent() ; ++j){
            evt = buffer.GetRingEvent()[i];
            timediff = (evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_ring->Fill(timediff, evt.ID);
        }

        for (j = 0 ; j < buffer.GetSectEvent() ; ++j){
            evt = buffer.GetSectEvent()[i];
            timediff = (evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_sect->Fill(timediff, evt.ID);
        }

        for (j = 0 ; j < buffer.GetBackEvent() ; ++j){
            evt = buffer.GetSectEvent()[i];
            timediff = (evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_back->Fill(timediff, evt.ID);
        }

        for (j = 0 ; j < buffer.GetLabrLEvent() ; ++j){
            evt = buffer.GetLabrLEvent()[i];
            timediff = (evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_labrL->Fill(timediff, evt.ID);
        }

        for (j = 0 ; j < buffer.GetLabrSEvent() ; ++j){
            evt = buffer.GetLabrSEvent()[i];
            timediff = (evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_labrS->Fill(timediff, evt.ID);
        }

        for (j = 0 ; j < buffer.GetLabrFEvent() ; ++j){
            evt = buffer.GetLabrFEvent()[i];
            timediff = (evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_labrF->Fill(timediff, evt.ID);
        }

        for (j = 0 ; j < buffer.GetCloverEvent() ; ++j){
            evt = buffer.GetCloverEvent()[i];
            timediff = (evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_clover->Fill(timediff, evt.ID);
        }
    }

    for (j = 0 ; j < buffer.GetRingEvent() ; ++j){
        evt = buffer.GetRingEvent()[i];
        energy_ring->Fill(evt.e_raw, evt.ID);
        energy_cal_ring->Fill(evt.energy, evt.ID);
    }

    for (j = 0 ; j < buffer.GetSectEvent() ; ++j){
        evt = buffer.GetSectEvent()[i];
        energy_sect->Fill(evt.e_raw, evt.ID);
        energy_cal_sect->Fill(evt.energy, evt.ID);
    }

    for (j = 0 ; j < buffer.GetBackEvent() ; ++j){
        evt = buffer.GetBackEvent()[i];
        energy_back->Fill(evt.e_raw, evt.ID);
        energy_cal_back->Fill(evt.energy, evt.ID);
    }

    for (j = 0 ; j < buffer.GetLabrLEvent() ; ++j){
        evt = buffer.GetLabrLEvent()[i];
        energy_labrL->Fill(evt.e_raw, evt.ID);
        energy_cal_labrL->Fill(evt.energy, evt.ID);
    }

    for (j = 0 ; j < buffer.GetLabrSEvent() ; ++j){
        evt = buffer.GetLabrSEvent()[i];
        energy_labrS->Fill(evt.e_raw, evt.ID);
        energy_cal_labrS->Fill(evt.energy, evt.ID);
    }

    for (j = 0 ; j < buffer.GetLabrFEvent() ; ++j){
        evt = buffer.GetLabrFEvent()[i];
        energy_labrF->Fill(evt.e_raw, evt.ID);
        energy_cal_labrF->Fill(evt.energy, evt.ID);
    }

    for (j = 0 ; j < buffer.GetCloverEvent() ; ++j){
        evt = buffer.GetCloverEvent()[i];
        energy_clover->Fill(evt.e_raw, evt.ID);
        energy_cal_clover->Fill(evt.energy, evt.ID);
    }

    for (i = 0 ; i < buffer.GetSectEvent() ; ++i){
        rfEvt = buffer.GetSectEvent()[i];
        for (j = 0 ; j < buffer.GetBackEvent() ; ++j){
            evt = buffer.GetBackEvent()[i];
            timediff = (evt.tcoarse - rfEvt.tcoarse);
            timediff += (evt.tfine - rfEvt.tfine);
            time_energy_sect_back->Fill(evt.energy, timediff);
        }
        for (j = 0 ; j < buffer.GetRingEvent() ; ++j) {
            evt = buffer.GetRingEvent()[i];
            if ( abs(rfEvt.energy - evt.energy) > 500 )
                continue;
            timediff = (evt.tcoarse - rfEvt.tcoarse);
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
