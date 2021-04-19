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


ROOT::HistManager::Detector_Histograms_t::Detector_Histograms_t(RootFileManager *fm, const std::string &name, const size_t &num)
    : time( fm->Mat(std::string("time_"+name), std::string("Time spectra "+name), 3000, -1500, 1500, "Time [ns]", num, 0, num, std::string(name+" ID").c_str()) )
    , energy( fm->Mat(std::string("energy_"+name), std::string("Energy spectra "+name), 65536, 0, 65536, "Energy [ch]", num, 0, num, std::string(name+" ID").c_str()) )
    , energy_cal( fm->Mat(std::string("energy_cal_"+name), std::string("energy spectra "+name+" (cal)"), 16384, 0, 16384, "Energy [keV]", num, 0, num, std::string(name+" ID").c_str()) )
    , mult( fm->Spec(std::string("mult_"+name), std::string("Multiplicity " + name), 128, 0, 128, "Multiplicity") )
{}

void ROOT::HistManager::Detector_Histograms_t::Fill(const word_t &word)
{
    auto dno = GetID(word.address);
    energy->Fill(word.adcdata, dno);
    energy_cal->Fill(CalibrateEnergy(word), dno);
}

void ROOT::HistManager::Detector_Histograms_t::Fill(const Subevent &subvec, const word_t *start)
{
    int dno;
    mult->Fill(subvec.size());
    for ( auto &entry : subvec ){
        dno = GetID(entry.address);
        energy->Fill(entry.adcdata, dno);
        energy_cal->Fill(entry.energy, dno);
        if ( start )
            time->Fill(double(entry.timestamp - start->timestamp) + (entry.cfdcorr - start->cfdcorr), dno);
    }
}


ROOT::HistManager::HistManager(RootFileManager *fm)
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

void ROOT::HistManager::AddEntry(Event &buffer)
{

    // LaBr 0 is our time reference. We only use it if there is only one of that type.
    const word_t *start = nullptr;
    int ref_num = 0;
    for ( auto &entry : buffer.GetDetector(DetectorType::labr_2x2_fs) ){
        if ( GetID(entry.address) == 0 ){
            ++ref_num;
            start = &entry;
        }
    }

    if (ref_num != 1)
        start = nullptr;

    for (auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, DetectorType::clover}){
        auto *hist = GetSpec(type);
        if ( hist )
            hist->Fill(buffer.GetDetector(type), start);
    }

    for ( auto sect_evt : buffer.GetDetector(DetectorType::de_sect) ){

        for ( auto ring_evt : buffer.GetDetector(DetectorType::de_ring) ){
            time_energy_ring_sect->Fill(
                    ring_evt.energy,
                    double(ring_evt.timestamp - sect_evt.timestamp) + (ring_evt.cfdcorr - sect_evt.cfdcorr));
        }

        for ( auto back_evt : buffer.GetDetector(DetectorType::eDet) ){
            time_energy_sect_back->Fill(
                    back_evt.energy,
                    double(back_evt.timestamp - sect_evt.timestamp) + (back_evt.cfdcorr - sect_evt.cfdcorr));
        }

    }
}

void ROOT::HistManager::AddEntry(const word_t &word)
{
    auto *spec = GetSpec(GetDetector(word.address).type);
    if ( spec )
        spec->Fill(word);
}

ROOT::HistManager::Detector_Histograms_t *ROOT::HistManager::GetSpec(const DetectorType &type)
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