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

#include "RootInterface/HistManager.h"
#include <Parameters/experimentsetup.h>

#include <TH1.h>
#include <TH2.h>
#include <Event/iThembaEvent.h>

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
    , addback_hist( fm->CreateTH2("time_self_clover", "Time spectra, clover self timing",3000, -1500, 1500, "Time [ns]",NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") )
{
}

#if ROOT_MT_FLAG
HistManager::HistManager(RootMergeFileManager *fm)
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
        , addback_hist( fm->CreateTH2("time_self_clover", "Time spectra, clover self timing",3000, -1500, 1500, "Time [ns]",NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") )
{
}
#endif // ROOT_MT_FLAG

void HistManager::FillTDiff(const Event::iThembaEntry &start, const std::vector<Event::iThembaEntry> &entries, TH2 *hist)
{
    double tdiff;
    for ( auto &stop : entries ){
        tdiff = double(stop.tcoarse  - start.tcoarse);
        tdiff += stop.tfine - start.tfine;
        hist->Fill(tdiff, stop.ID);
    }
}

void HistManager::FillEnergy(const std::vector<Event::iThembaEntry> &entries, TH2 *hist, TH2 *hist_cal)
{
    for ( auto &entry : entries ){
        hist->Fill(entry.e_raw, entry.ID);
        hist_cal->Fill(entry.energy, entry.ID);
    }
}


void HistManager::AddEntry(const Event::iThembaEvent &event)
{
    // First time spectra. We use the RF as reference.


    for ( auto &rfEntry : event.GetLabrF() ){
        if ( rfEntry.ID != 0 )
            continue;

        FillTDiff(rfEntry, event.GetRing(), time_ring);
        FillTDiff(rfEntry, event.GetSect(), time_sect);
        FillTDiff(rfEntry, event.GetBack(), time_back);
        FillTDiff(rfEntry, event.GetLabrL(), time_labrL);
        FillTDiff(rfEntry, event.GetLabrS(), time_labrS);
        FillTDiff(rfEntry, event.GetLabrF(), time_labrF);
        FillTDiff(rfEntry, event.GetClover(), time_clover);
    }

    FillEnergy(event.GetRing(), energy_ring, energy_cal_ring);
    FillEnergy(event.GetSect(), energy_sect, energy_cal_sect);
    FillEnergy(event.GetBack(), energy_back, energy_cal_back);
    FillEnergy(event.GetLabrL(), energy_labrL, energy_cal_labrL);
    FillEnergy(event.GetLabrS(), energy_labrS, energy_cal_labrS);
    FillEnergy(event.GetLabrF(), energy_labrF, energy_cal_labrF);
    FillEnergy(event.GetClover(), energy_clover, energy_cal_clover);

}
