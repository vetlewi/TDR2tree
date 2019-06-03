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

#ifndef HISTMANAGER_H
#define HISTMANAGER_H

#include "RootFileManager.h"

class Event;

class HistManager {

private:

    //! Time spectra for each detector
    TH2 *time_ring;     //!< Alignment spectra of dE rings.
    TH2 *time_sect;     //!< Alignment spectra of dE sectors.
    TH2 *time_back;     //!< Alignment spectra of dE back detectors.
    TH2 *time_labrL;    //!< Alignment spectra of large LaBr detectors.
    TH2 *time_labrS;    //!< Alignment spectra of small LaBr detectors (slow signal).
    TH2 *time_labrF;    //!< Alignment spectra of small LaBr detectors (fast signal).
    TH2 *time_clover;   //!< Alignment spectra of clover crystals.

    //! Time energy spectra for particles.
    TH2 *time_energy_sect_back;
    TH2 *time_energy_ring_sect;

    //! Energy spectra for each detector.
    TH2 *energy_ring;
    TH2 *energy_sect;
    TH2 *energy_back;
    TH2 *energy_labrL;
    TH2 *energy_labrS;
    TH2 *energy_labrF;
    TH2 *energy_clover;

    TH2 *energy_cal_ring;
    TH2 *energy_cal_sect;
    TH2 *energy_cal_back;
    TH2 *energy_cal_labrL;
    TH2 *energy_cal_labrS;
    TH2 *energy_cal_labrF;
    TH2 *energy_cal_clover;


public:

    //! Constructor.
    explicit HistManager(RootFileManager *fileManager    /*!< ROOT file where the histograms will reside.    */);

    //! Fill spectra with an event.
    void AddEntry(const Event &buffer  /*!< Event to read from    */);

    //! Fill spectra with a list of events.
    void AddEntries(const std::vector<Event> &evts);

};


#endif // HISTMANAGER_H
