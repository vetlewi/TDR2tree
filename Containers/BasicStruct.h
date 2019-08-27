#ifndef BASICSTRUCT_H
#define BASICSTRUCT_H

#include <cstdint>


typedef struct {
    uint16_t address;		//!< Holds the address of the ADC.
    uint16_t adcdata;		//!< Data read out from the ADC.
    uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
    char cfdfail;           //!< Flag to tell if the CFD was forced or not.
    char finishcode;        //!< Pile-up flag.
    int64_t timestamp;		//!< Timestamp in [ns].
    double cfdcorr;         //!< Correction from the CFD.
} word_t;

#endif // BASICSTRUCT_H
