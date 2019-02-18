#ifndef BASICSTRUCT_H
#define BASICSTRUCT_H

#include <cstdint>
#include <vector>

#include "experimentsetup.h"

#define NUM_MAX 32

class TH1;
class TH2;
class TDirectory;
class HistogramManager;

typedef struct {
    uint16_t address;		//!< Holds the address of the ADC.
    uint16_t adcdata;		//!< Data read out from the ADC.
    uint16_t cfddata;       //!< Fractional difference of before/after zero-crossing.
    char cfdfail;           //!< Flag to tell if the CFD was forced or not.
    char finishcode;        //!< Pile-up flag.
    int64_t timestamp;		//!< Timestamp in [ns].
    double cfdcorr;         //!< Correction from the CFD.
} word_t;

struct Options {
    bool make_tree;
    bool use_addback;
    bool use_all_labrS;
};

struct Event {

    // Fields for the ring
    int ring_mult;
    int16_t ringID[NUM_MAX];
    double ring_energy[NUM_MAX], ring_t_fine[NUM_MAX];
    int64_t ring_t_coarse[NUM_MAX];

    // Fields for the sectors
    int sect_mult;
    int16_t sectID[NUM_MAX];
    double sect_energy[NUM_MAX], sect_t_fine[NUM_MAX];
    int64_t sect_t_coarse[NUM_MAX];

    // Fields for the back detector
    int back_mult;
    int16_t backID[NUM_MAX];
    double back_energy[NUM_MAX], back_t_fine[NUM_MAX];
    int64_t back_t_coarse[NUM_MAX];

    // Fields for the labr L
    int labrL_mult;
    int16_t labrLID[NUM_MAX];
    double labrL_energy[NUM_MAX], labrL_t_fine[NUM_MAX];
    int64_t labrL_t_coarse[NUM_MAX];

    // Fields for the labr S
    int labrS_mult;
    int16_t labrSID[NUM_MAX];
    double labrS_energy[NUM_MAX], labrS_t_fine[NUM_MAX];
    int64_t labrS_t_coarse[NUM_MAX];

    // Fields for the clover
    int clover_mult;
    int16_t cloverID[NUM_MAX];
    int16_t clover_crystal[NUM_MAX]; // We need to know which crystal is hit first.
    double clover_energy[NUM_MAX], clover_t_fine[NUM_MAX];
    int64_t clover_t_coarse[NUM_MAX];

    inline void Reset()
    {
        ring_mult = 0;
        sect_mult = 0;
        back_mult = 0;
        labrL_mult = 0;
        labrS_mult = 0;
        clover_mult = 0;
    }

    inline void AddRing(const int16_t &id, const double &energy, const int64_t &coarse, const double &fine)
    {
        if (ring_mult < NUM_MAX){
            ringID[ring_mult] = id;
            ring_energy[ring_mult] = energy;
            ring_t_coarse[ring_mult] = coarse;
            ring_t_fine[ring_mult++] = fine;
        }
    }

    inline void AddSect(const int16_t &id, const double &energy, const int64_t &coarse, const double &fine)
    {
        if (sect_mult < NUM_MAX){
            sectID[sect_mult] = id;
            sect_energy[sect_mult] = energy;
            sect_t_coarse[sect_mult] = coarse;
            sect_t_fine[sect_mult++] = fine;
        }
    }

    inline void AddBack(const int16_t &id, const double &energy, const int64_t &coarse, const double &fine)
    {
        if (back_mult < NUM_MAX){
            backID[back_mult] = id;
            back_energy[back_mult] = energy;
            back_t_coarse[back_mult] = coarse;
            back_t_fine[back_mult++] = fine;
        }
    }

    inline void AddLabrL(const int16_t &id, const double &energy, const int64_t &coarse, const double &fine)
    {
        if (labrL_mult < NUM_MAX){
            labrLID[labrL_mult] = id;
            labrL_energy[labrL_mult] = energy;
            labrL_t_coarse[labrL_mult] = coarse;
            labrL_t_fine[labrL_mult++] = fine;
        }
    }

    inline void AddLabrS(const int16_t &id, const double &energy, const int64_t &coarse, const double &fine)
    {
        if (labrS_mult < NUM_MAX){
            labrSID[labrS_mult] = id;
            labrS_energy[labrS_mult] = energy;
            labrS_t_coarse[labrS_mult] = coarse;
            labrS_t_fine[labrS_mult++] = fine;
        }
    }

    inline void AddClover(const int16_t &id, const double &energy, const int64_t &coarse, const double &fine)
    {
        if (clover_mult < NUM_MAX){
            cloverID[clover_mult] = id;
            clover_energy[clover_mult] = energy;
            clover_t_coarse[clover_mult] = coarse;
            clover_t_fine[clover_mult++] = fine;
        }
    }

    void RunAddback(TH2 *ab_t_clover /*!< Matrix to fill in order to set propper time gates for clover addback */);

};

class HistogramManager
{
public:
    HistogramManager();
    virtual ~HistogramManager();

    void Fill(const Event *event, const Options &opt);

    TH2 *GetAB(){ return time_self_clover; } /*!< Return Addback time spectrum */

private:

    //! Time spectra for each detector
    TH2 *time_ring;     //!< Alignment spectra of dE rings.
    TH2 *time_sect;     //!< Alignment spectra of dE sectors.
    TH2 *time_back;     //!< Alignment spectra of dE back detectors.
    TH2 *time_labrL;    //!< Alignment spectra of large LaBr detectors.
    TH2 *time_labrS;    //!< Alignment spectra of small LaBr detectors.
    TH2 *time_clover;   //!< Alignment spectra of clover crystals.
    TH2 *time_self_clover;  //!< Self time difference (for addback).

    //! Time-energy spectra
    /*TH2 *time_energy_ring;
    TH2 *time_energy_sect;
    TH2 *time_energy_back;
    TH2 *time_energy_labrL;
    TH2 *time_energy_labrS;
    TH2 *time_energy_clover;*/

    TH1 *Spec(const char *name, const char *title, const int &nbins, const double &xmin, const double &xmax, const char *xtitle, const char *ytitle);
    TH2 *Mat(const char *name, const char *title, const int &xbins, const double &xmin, const double &xmax, const char *xtitle,
                                                  const int &ybins, const double &ymin, const double &ymac, const char *ytitle);
};

#endif // BASICSTRUCT_H
