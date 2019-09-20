#ifndef BASICSTRUCT_H
#define BASICSTRUCT_H

#include <cstdint>


struct word_t {
    uint16_t address;       //!< XIA address of the entry.
    uint16_t adcdata;		//!< Data read out from the ADC.
    uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
    char cfdfail;           //!< Flag to tell if the CFD was forced or not.
    char finishcode;        //!< Pile-up flag.
    int64_t timestamp;		//!< Timestamp in [ns].
    double cfdcorr;         //!< Correction from the CFD.
    double energy;          //!< Calibrated energy [keV].
};

extern bool operator<(const word_t &lhs, const word_t &rhs);

#endif // BASICSTRUCT_H
