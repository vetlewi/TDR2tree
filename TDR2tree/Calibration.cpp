#include "Calibration.h"

#include "experimentsetup.h"

#include "Parameters.h"

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <cmath>
#include <cstdint>

static Parameters calParam;

//! Parameters for energy calibration of the LaBr detectors
static Parameter gain_labrL(calParam, "gain_labrL", NUM_LABR_3X8_DETECTORS, 1);
static Parameter shift_labrL(calParam, "shift_labrL", NUM_LABR_3X8_DETECTORS, 0);
static Parameter gain_labrS(calParam, "gain_labrS", NUM_LABR_2X2_DETECTORS, 1);
static Parameter shift_labrS(calParam, "shift_labrS", NUM_LABR_3X8_DETECTORS, 0);

//! Parameters for energy calibration of the CLOVER detectors
static Parameter gain_clover(calParam, "gain_clover", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 1);
static Parameter shift_clover(calParam, "shift_clover", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0);

//! Parameters for energy calibration of the Si detectors
static Parameter gain_ring(calParam, "gain_ring", NUM_SI_RING, 1);
static Parameter shift_ring(calParam, "shift_ring", NUM_SI_RING, 0);
static Parameter gain_sect(calParam, "gain_sect", NUM_SI_SECT, 1);
static Parameter shift_sect(calParam, "shift_sect", NUM_SI_SECT, 0);
static Parameter gain_back(calParam, "gain_back", NUM_SI_BACK, 1);
static Parameter shift_back(calParam, "shift_back", NUM_SI_BACK, 1);

//! Alignment parameters for time
static Parameter shift_t_labrL(calParam, "shift_t_labrL", NUM_LABR_3X8_DETECTORS, 0);
static Parameter shift_t_labrS(calParam, "shift_t_labrS", NUM_LABR_2X2_DETECTORS, 0);
static Parameter shift_t_clover(calParam, "shift_t_clover", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0);
static Parameter shift_t_ring(calParam, "shift_t_ring", NUM_SI_RING, 0);
static Parameter shift_t_sect(calParam, "shift_t_sect", NUM_SI_SECT, 0);
static Parameter shift_t_back(calParam, "shift_t_back", NUM_SI_BACK, 0);

//! Time gate for addback in clover detectors
static Parameter clover_addback_gate(calParam, "clover_addback_gate", 2, 0);

//! From the input parameters we will construct TimeType calibration.
/*static Event_time_t timecal_labrL[NUM_LABR_3X8_DETECTORS];
static Event_time_t timecal_labrS[NUM_LABR_2X2_DETECTORS];
static Event_time_t timecal_clover[NUM_CLOVER_DETECTORS][NUM_CLOVER_CRYSTALS];
static Event_time_t timecal_ring[NUM_SI_RING];
static Event_time_t timecal_sect[NUM_SI_SECT];
static Event_time_t timecal_back[NUM_SI_BACK];
static bool timecal_set = false;

void SetTimeCal(Event_time_t *dest, Parameter &orig, size_t length)
{
    double frac, whole;
    for (size_t i = 0 ; i < length ; ++i){
        frac = std::modf(orig[i], &whole);
        dest[i] = {static_cast<int64_t>(whole), frac};
    }
}

void BuildTimeCal()
{
    double frac, whole;
    SetTimeCal(timecal_labrL, shift_t_labrL, NUM_LABR_3X8_DETECTORS);
    SetTimeCal(timecal_labrS, shift_t_labrS, NUM_LABR_2X2_DETECTORS);
    SetTimeCal(timecal_ring, shift_t_ring, NUM_SI_RING);
    SetTimeCal(timecal_sect, shift_t_sect, NUM_SI_RING);
    SetTimeCal(timecal_back, shift_t_back, NUM_SI_RING);
    for (int i = 0 ; i < NUM_CLOVER_DETECTORS ; ++i){
        for (int j = 0 ; j < NUM_CLOVER_CRYSTALS ; ++j){
            frac = std::modf(shift_t_clover[i*NUM_CLOVER_CRYSTALS + j], &whole);
            timecal_clover[i][j] = Event_time_t(static_cast<int64_t>(whole), frac);
        }
    }
    timecal_set = true;
}*/

bool NextLine(std::istream &in, std::string &outline, int &lineno)
{
    outline = "";
    std::string line;
    while ( getline(in, line) ){
        lineno++;
        size_t ls = line.size();
        if ( ls == 0 ){
            break;
        } else if ( line[ls - 1] != '\\' ){
            outline += line;
            break;
        } else {
            outline += line.substr(0, ls-1);
        }
    }
    return in || !line.empty();
}

bool SetCalibration(const char *calfile)
{
    // Open file
    std::ifstream inCal(calfile);
    std::string currentLine;
    int lineno = 0; // Keep track of line read. To make it easier to debug :)

    // Get line by line!
    while ( NextLine(inCal, currentLine, lineno) ){
        std::istringstream icmd(currentLine);
        if ( !calParam.SetAll(icmd) ){
            std::cerr << "Error extracting calibration from line ";
            std::cerr << lineno << " in '" << calfile << "': ";
            std::cerr << currentLine << std::endl;
            return false;
        }
    }
    // Make sure we have time calibration on the correct format.
    //BuildTimeCal();
    return true;
}

double CalibrateEnergy(const word_t &detector)
{
    DetectorInfo_t dinfo = GetDetector(detector.address);
    switch (dinfo.type) {
        case labr_3x8 :
            return gain_labrL[dinfo.detectorNum]*detector.adcdata + shift_labrL[dinfo.detectorNum];
        case labr_2x2 :
            return gain_labrS[dinfo.detectorNum]*detector.adcdata + shift_labrS[dinfo.detectorNum];
        case clover :
            return gain_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS+dinfo.telNum]*detector.adcdata + shift_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS+dinfo.telNum];
        case de_ring :
            return gain_ring[dinfo.detectorNum]*detector.adcdata + shift_ring[dinfo.detectorNum];
        case de_sect :
            return gain_sect[dinfo.detectorNum]*detector.adcdata + shift_sect[dinfo.detectorNum];
        case eDet :
            return gain_back[dinfo.detectorNum]*detector.adcdata + shift_back[dinfo.detectorNum];
        default :
            return detector.adcdata;
    }
}

double CalibrateTime(const word_t &detector)
{
    DetectorInfo_t dinfo = GetDetector(detector.address);
    switch (dinfo.type) {
        case labr_3x8 :
            return detector.cfdcorr + shift_t_labrL[dinfo.detectorNum];
        case labr_2x2 :
            return detector.cfdcorr + shift_t_labrS[dinfo.detectorNum];
        case clover :
            return detector.cfdcorr + shift_t_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS + dinfo.telNum];
        case de_ring :
            return detector.cfdcorr + shift_t_ring[dinfo.detectorNum];
        case de_sect :
            return detector.cfdcorr + shift_t_sect[dinfo.detectorNum];
        case eDet :
            return detector.cfdcorr + shift_t_back[dinfo.detectorNum];
        default :
            return detector.cfdcorr;
    }
}

/*Event_time_t CalibrateTime(const word_t &detector)
{   
    Event_time_t timestamp = {detector.timestamp, detector.cfdcorr};
    DetectorInfo_t dinfo = GetDetector(detector.address);
    switch (dinfo.type) {
        case labr_3x8 :
            timestamp -= timecal_labrL[dinfo.detectorNum];
            break;
        case labr_2x2 :
            timestamp -= timecal_labrS[dinfo.detectorNum];
            break;
        case clover :
            timestamp -= timecal_clover[dinfo.detectorNum][dinfo.telNum];
            break;
        case de_ring :
            timestamp -= timecal_ring[dinfo.detectorNum];
            break;
        case de_sect :
            timestamp -= timecal_sect[dinfo.detectorNum];
            break;
        case eDet :
            timestamp -= timecal_back[dinfo.detectorNum];
            break;
        default :
            break;
    }
    return timestamp;
}*/


bool CheckTimeGateAddback(const double &timediff)
{
   return (timediff >= clover_addback_gate[0] && timediff <= clover_addback_gate[1]) ? true : false;
}
