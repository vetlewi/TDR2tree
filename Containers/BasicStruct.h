#ifndef BASICSTRUCT_H
#define BASICSTRUCT_H

#include <cstdint>


typedef struct {
    uint16_t crateID;       //!< Crate address of the ADC.
    uint16_t slotID;        //!< Slot address of the ADC.
    uint16_t chanID;        //!< Channel # of the ADC.
    uint16_t adcdata;		//!< Data read out from the ADC.
    uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
    char cfdfail;           //!< Flag to tell if the CFD was forced or not.
    char finishcode;        //!< Pile-up flag.
    int64_t timestamp;		//!< Timestamp in [ns].
    double cfdcorr;         //!< Correction from the CFD.
    double energy;          //!< Calibrated energy [keV].
} word_t;

extern bool operator<(const word_t &lhs, const word_t &rhs);

#endif // BASICSTRUCT_H
