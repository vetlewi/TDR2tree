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
struct EventData;
struct EventEntry;

class HistManager {

private:

    struct Detector_Histograms_t {
        Histogram2Dp time;          //! Time alignment spectra
        Histogram2Dp energy;
        Histogram2Dp energy_cal;
        Histogram1Dp mult;

        Detector_Histograms_t(RootFileManager *fileManager, const std::string &name, const size_t &num);

        void Fill(const EventData &events);
        void Fill(const EventData &events, const EventEntry &start);
    };

    Detector_Histograms_t ring;
    Detector_Histograms_t sect;
    Detector_Histograms_t back;

    Detector_Histograms_t labrL;
    Detector_Histograms_t labrS;
    Detector_Histograms_t labrF;
    Detector_Histograms_t clover;

    //! Time energy spectra for particles.
    Histogram2Dp time_energy_sect_back;
    Histogram2Dp time_energy_ring_sect;


public:

    //! Constructor.
    explicit HistManager(RootFileManager *fileManager    /*!< ROOT file where the histograms will reside.    */);

    //! Fill spectra with an event.
    void AddEntry(const Event &buffer  /*!< Event to read from    */);

    //! Fill spectra with a list of events.
    void AddEntries(const std::vector<Event> &evts);

};


#endif // HISTMANAGER_H
