//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#ifndef EVENT_H
#define EVENT_H

#include <cstdint>

namespace Parser {

    typedef struct {
        uint16_t address;       //!< XIA address of the entry.
        uint16_t adcdata;		//!< Data read out from the ADC.
        uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
        int64_t timestamp;		//!< Timestamp in [ns].
        double cfdcorr;         //!< Correction from the CFD.
        double energy;          //!< Calibrated energy [keV].
        bool cfdfail;           //!< Flag to tell if the CFD was forced or not.
        bool finishcode;        //!< Pile-up flag.
    } Entry_t;

}

#endif // EVENT_H
