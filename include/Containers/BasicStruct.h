#ifndef BASICSTRUCT_H
#define BASICSTRUCT_H

#include <cstdint>


struct word_t {
    uint16_t address;		//!< Holds the address of the ADC.
    uint16_t adcdata;		//!< Data read out from the ADC.
    uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
    bool finishcode;        //!< Pile-up flag.
    bool veto;              //!< Veto flag
    int64_t timestamp;		//!< Timestamp in [ns].
    int64_t timestamp_raw;  //!< Raw timestamp in [ns]. Before any alignment
    double energy;          //!< Calibrated energy [keV]
    bool cfdfail;           //!< Flag to tell if the CFD was forced or not.
    double cfdcorr;         //!< Correction from the CFD [ns]. Before any alignment
};

#endif // BASICSTRUCT_H
