#ifndef EXPERIMENTSETUP_H
#define EXPERIMENTSETUP_H

// Currently the sorting rutine will only support dE-E silicon telescopes.
// This may change in the future if needed... I think...

#define NUM_CLOVER_DETECTORS 10     //!< Number of Clover detectors
#define NUM_CLOVER_CRYSTALS 4       //!< Number of Clover crystals per detector
#define NUM_LABR_3X8_DETECTORS 2    //!< Number of LaBr detectors
#define NUM_LABR_2X2_DETECTORS 6    //!< Number of LaBr detectors
#define NUM_SI_RING 48           //!< Number of Si dE rings
#define NUM_SI_SECT 16           //!< Number of Si dE sector
#define NUM_SI_BACK 16             //!< Number of Si  E sector

#define MAX_WORDS_PER_DET 32    //!< Maximum number of words per detector in each event


#define TOTAL_NUMBER_OF_ADDRESSES 545   //! Total number of address that needs to be defined

#if __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

enum DetectorType {
    invalid,        //!< Invalid address
    labr_3x8,       //!< Is a 3.5x8 inch labr detector
    labr_2x2_ss,    //!< Is a 2x2 labr detector, slow signal
    labr_2x2_fs,    //! Is a 2x2 labr detector, fast signal
    clover,         //!< Is a clover crystal
    de_ring,        //!< Is a Delta-E ring
    de_sect,        //!< Is a Delta-E sector
    eDet,           //!< Is a E detector
    rfchan,         //! The channel where the RF is connected
    unused          //!< Is a unused XIA channel
};

enum ADCSamplingFreq {
    f100MHz,    //!< 100 MHz sampling frequency
    f250MHz,    //!< 250 MHz sampling frequency
    f500MHz,    //!< 500 MHz sampling frequency
    f000MHz     //!< If invalid address
};

struct DetectorInfo_ {
    uint16_t address;           //!< ADC address of the detector
    enum ADCSamplingFreq sfreq; //!< ADC sampling frequency
    enum DetectorType type;     //!< Type of detector
    int16_t detectorNum;            //!< 'Linear' number of the detector
    int16_t telNum;                 //!< Telescope number (ie. E back detector for the dE front detector)
};

typedef struct DetectorInfo_ DetectorInfo_t;

//! Get detector method
/*! \return Detector structure containing information about the
 *  detector at address.
 */
DetectorInfo_t GetDetector(uint16_t address   /*!< Address of the detector to get */);

//! Get sampling frequency
/*! \return The XIA module sampling frequency
 */
enum ADCSamplingFreq GetSamplingFrequency(uint16_t address    /*!< ADC address    */);

#if __cplusplus
}
#endif // __cplusplus



#endif // EXPERIMENTSETUP_H
