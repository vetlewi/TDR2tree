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
    , energy_ring( fm->CreateTH2("energy_ring", "Energy spectra rings", 16384, 0, 16384, "Energy [ch]", NUM_SI_RING, 0, NUM_SI_RING, "Ring ID") )
    , energy_sect( fm->CreateTH2("energy_sect", "Energy spectra sectors", 16384, 0, 16384, "Energy [ch]", NUM_SI_SECT, 0, NUM_SI_SECT, "Sector ID") )
    , energy_back( fm->CreateTH2("energy_back", "Energy spectra back detectors", 16384, 0, 16384, "Energy [ch]", NUM_SI_BACK, 0, NUM_SI_BACK, "Back ID") )
    , energy_labrL( fm->CreateTH2("energy_labrL", "Energy spectra LaBr L", 16384, 16384, 16384, "Energy [ch]", NUM_LABR_3X8_DETECTORS, 0, NUM_LABR_3X8_DETECTORS, "LaBr L ID") )
    , energy_labrS( fm->CreateTH2("energy_labrS", "Energy spectra LaBr S", 16384, 16384, 16384, "Energy [ch]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr S ID") )
    , energy_labrF( fm->CreateTH2("energy_labrF", "Energy spectra LaBr F", 16384, 16384, 16384, "Energy [ch]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr F ID") )
    , energy_clover( fm->CreateTH2("energy_clover", "Energy spectra CLOVER", 16384, 16384, 16384, "Energy [ch]", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0, NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, "CLOVER ID") )
    , energy_cal_ring( fm->CreateTH2("energy_cal_ring", "Energy spectra rings", 16384, 0, 16384, "Energy [keV]", NUM_SI_RING, 0, NUM_SI_RING, "Ring ID") )
    , energy_cal_sect( fm->CreateTH2("energy_cal_sect", "Energy spectra sectors", 16384, 0, 16384, "Energy [keV]", NUM_SI_SECT, 0, NUM_SI_SECT, "Sector ID") )
    , energy_cal_back( fm->CreateTH2("energy_cal_back", "Energy spectra back detectors", 16384, 0, 16384, "Energy [keV]", NUM_SI_BACK, 0, NUM_SI_BACK, "Back ID") )
    , energy_cal_labrL( fm->CreateTH2("energy_cal_labrL", "Energy spectra LaBr L", 16384, 16384, 16384, "Energy [keV]", NUM_LABR_3X8_DETECTORS, 0, NUM_LABR_3X8_DETECTORS, "LaBr L ID") )
    , energy_cal_labrS( fm->CreateTH2("energy_cal_labrS", "Energy spectra LaBr S", 16384, 16384, 16384, "Energy [keV]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr S ID") )
    , energy_cal_labrF( fm->CreateTH2("energy_cal_labrF", "Energy spectra LaBr F", 16384, 16384, 16384, "Energy [keV]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "LaBr F ID") )
    , energy_cal_clover( fm->CreateTH2("energy_cal_clover", "Energy spectra CLOVER", 16384, 16384, 16384, "Energy [keV]", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0, NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, "CLOVER ID") )
{

}

void HistManager::AddEntry(const Event &buffer)
{
    // First time spectra. We use the RF as reference.
    double timediff;
    for ( auto rfEvt : buffer.GetRFEvent() ){

        for ( auto ringEvt : buffer.GetRingEvent() ){
            timediff = (ringEvt.tcoarse - rfEvt.tcoarse);
            timediff += (ringEvt.tfine - rfEvt.tfine);
            time_ring->Fill(timediff, ringEvt.ID);
        }

        for ( auto sectEvt : buffer.GetSectEvent() ){
            timediff = (sectEvt.tcoarse - rfEvt.tcoarse);
            timediff += (sectEvt.tfine - rfEvt.tfine);
            time_sect->Fill(timediff, sectEvt.ID);
        }

        for ( auto backEvt : buffer.GetBackEvent() ){
            timediff = (backEvt.tcoarse - rfEvt.tcoarse);
            timediff += (backEvt.tfine - rfEvt.tfine);
            time_back->Fill(timediff, backEvt.ID);
        }

        for ( auto labrLEvt : buffer.GetLabrLEvent() ){
            timediff = (labrLEvt.tcoarse - rfEvt.tcoarse);
            timediff += (labrLEvt.tfine - rfEvt.tfine);
            time_labrL->Fill(timediff, labrLEvt.ID);
        }

        for ( auto labrSEvt : buffer.GetLabrSEvent() ){
            timediff = (labrSEvt.tcoarse - rfEvt.tcoarse);
            timediff += (labrSEvt.tfine - rfEvt.tfine);
            time_labrS->Fill(timediff, labrSEvt.ID);
        }

        for ( auto labrFEvt : buffer.GetLabrFEvent() ){
            timediff = (labrFEvt.tcoarse - rfEvt.tcoarse);
            timediff += (labrFEvt.tfine - rfEvt.tfine);
            time_labrF->Fill(timediff, labrFEvt.ID);
        }

        for ( auto cloverEvt : buffer.GetCloverEvent() ){
            timediff = (cloverEvt.tcoarse - rfEvt.tcoarse);
            timediff += (cloverEvt.tfine - rfEvt.tfine);
            time_clover->Fill(timediff, cloverEvt.ID);
        }
    }

    for ( auto ringEvt : buffer.GetRingEvent() ){
        energy_ring->Fill(ringEvt.e_raw, ringEvt.ID);
        energy_cal_ring->Fill(ringEvt.energy, ringEvt.ID);
    }

    for ( auto sectEvt : buffer.GetSectEvent() ){
        energy_sect->Fill(sectEvt.e_raw, sectEvt.ID);
        energy_cal_sect->Fill(sectEvt.energy, sectEvt.ID);
    }

    for ( auto backEvt : buffer.GetBackEvent() ){
        energy_back->Fill(backEvt.e_raw, backEvt.ID);
        energy_cal_back->Fill(backEvt.energy, backEvt.ID);
    }

    for ( auto labrLEvt : buffer.GetLabrLEvent() ){
        energy_labrL->Fill(labrLEvt.e_raw, labrLEvt.ID);
        energy_cal_labrL->Fill(labrLEvt.energy, labrLEvt.ID);
    }

    for ( auto labrSEvt : buffer.GetLabrSEvent() ){
        energy_labrS->Fill(labrSEvt.e_raw, labrSEvt.ID);
        energy_cal_labrS->Fill(labrSEvt.energy, labrSEvt.ID);
    }

    for ( auto labrFEvt : buffer.GetLabrFEvent() ){
        energy_labrF->Fill(labrFEvt.e_raw, labrFEvt.ID);
        energy_cal_labrF->Fill(labrFEvt.energy, labrFEvt.ID);
    }

    for ( auto cloverEvt : buffer.GetCloverEvent() ){
        energy_clover->Fill(cloverEvt.e_raw, cloverEvt.ID);
        energy_cal_clover->Fill(cloverEvt.energy, cloverEvt.ID);
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
