#include "Parameters/Calibration.h"
#include "Parameters/experimentsetup.h"
#include "Parameters/Parameters.h"
#include "Parameters/XIA_CFD.h"

#include <Parser/Entry.h>

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

static Parameters calParam;

//! Parameters for energy calibration of the LaBr detectors
static Parameter gain_labrL(calParam, "gain_labrL", NUM_LABR_3X8_DETECTORS, 1);
static Parameter shift_labrL(calParam, "shift_labrL", NUM_LABR_3X8_DETECTORS, 0);
static Parameter gain_labrS(calParam, "gain_labrS", NUM_LABR_2X2_DETECTORS, 1);
static Parameter shift_labrS(calParam, "shift_labrS", NUM_LABR_2X2_DETECTORS, 0);
static Parameter gain_labrF(calParam, "gain_labrF", NUM_LABR_2X2_DETECTORS, 1);
static Parameter shift_labrF(calParam, "shift_labrF", NUM_LABR_2X2_DETECTORS, 0);

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
static Parameter shift_t_labrF(calParam, "shift_t_labrF", NUM_LABR_2X2_DETECTORS, 0);
static Parameter shift_t_clover(calParam, "shift_t_clover", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0);
static Parameter shift_t_ring(calParam, "shift_t_ring", NUM_SI_RING, 0);
static Parameter shift_t_sect(calParam, "shift_t_sect", NUM_SI_SECT, 0);
static Parameter shift_t_back(calParam, "shift_t_back", NUM_SI_BACK, 0);

//! Time gate for addback in clover detectors
static Parameter clover_addback_gate(calParam, "clover_addback_gate", 2, 0);


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

double CalibrateEnergy(const Parser::Entry_t &detector)
{
    double energy = 0;
    DetectorInfo_t dinfo = GetDetector(detector.address);
    switch (dinfo.type) {
        case labr_3x8 :
            energy = gain_labrL[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_labrL[dinfo.detectorNum];
            break;
        case labr_2x2_ss :
            energy = gain_labrS[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_labrS[dinfo.detectorNum];
            break;
        case labr_2x2_fs :
            energy = gain_labrF[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_labrF[dinfo.detectorNum];
            break;
        case clover :
            energy = gain_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS+dinfo.telNum]*(detector.adcdata + drand48() - 0.5) + shift_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS+dinfo.telNum];
            break;
        case de_ring :
            energy = gain_ring[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_ring[dinfo.detectorNum];
            break;
        case de_sect :
            energy = gain_sect[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_sect[dinfo.detectorNum];
            break;
        case eDet :
            energy = gain_back[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_back[dinfo.detectorNum];
            break;
        default :
            energy = detector.adcdata;
            break;
    }
    return energy;
}

double CalibrateCFD(const Parser::Entry_t &detector, int64_t &timestamp, bool &cfdfail)
{
    double cfdcorr = 0;
    switch ( GetSamplingFrequency(detector.address) ){
        case f000MHz :
            cfdfail = true;
            break;
        case f100MHz :
            cfdcorr = XIA_CFD_Fraction_100MHz(detector.cfddata, cfdfail);
            timestamp *= 10;
            if ( detector.cfddata == 0 )
                cfdfail = true;
            break;
        case f250MHz :
            cfdcorr = XIA_CFD_Fraction_250MHz(detector.cfddata, cfdfail);
            timestamp *= 8;
            if ( detector.cfddata == 0 )
                cfdfail = true;
            break;
        case f500MHz :
            cfdcorr = XIA_CFD_Fraction_500MHz(detector.cfddata, cfdfail);
            timestamp *= 10;
            if ( detector.cfddata == 0 )
                cfdfail = true;
            break;
    }
    return cfdcorr;
}

/*Parser::Entry_t &CalibrateCFD(Parser::Entry_t &detector)
{
    switch ( GetSamplingFrequency(detector.address) ) {
        case f100MHz :
            detector.cfdcorr = XIA_CFD_Fraction_100MHz(detector.cfddata, detector.cfdfail);
            detector.timestamp *= 10;
            if ( detector.cfddata == 0 )
                detector.cfdfail = true;
            break;
        case f250MHz :
            detector.cfdcorr = XIA_CFD_Fraction_250MHz(detector.cfddata, detector.cfdfail);
            detector.timestamp *= 8;
            if ( detector.cfddata == 0 )
                detector.cfdfail = true;
            break;
        case f500MHz :
            detector.cfdcorr = XIA_CFD_Fraction_500MHz(detector.cfddata, detector.cfdfail);
            detector.timestamp *= 10;
            if ( detector.cfddata == 0 )
                detector.cfdfail = true;
            break;
        default :
            detector.cfdcorr = 0;
            detector.cfdfail = true;
            detector.timestamp *= 10;
            break;
    }
    return detector;
}*/

double CalTime(const Parser::Entry_t &detector)
{
    DetectorInfo_t dinfo = GetDetector(detector.address);
    double time = 0;
    switch (dinfo.type) {
        case labr_3x8 :
            time = detector.cfdcorr + shift_t_labrL[dinfo.detectorNum];
            break;
        case labr_2x2_ss :
            time = detector.cfdcorr + shift_t_labrS[dinfo.detectorNum];
            break;
        case labr_2x2_fs :
            time = detector.cfdcorr + shift_t_labrF[dinfo.detectorNum];
            break;
        case clover :
            time = detector.cfdcorr + shift_t_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS + dinfo.telNum];
            break;
        case de_ring :
            time = detector.cfdcorr + shift_t_ring[dinfo.detectorNum];
            break;
        case de_sect :
            time = detector.cfdcorr + shift_t_sect[dinfo.detectorNum];
            break;
        case eDet :
            time = detector.cfdcorr + shift_t_back[dinfo.detectorNum];
            break;
        default :
            time = detector.cfdcorr;
            break;
    }
    return time;
}

Parser::Entry_t &Calibrate(Parser::Entry_t &entry)
{
    entry.energy = CalibrateEnergy(entry);
    entry.cfdcorr = CalibrateCFD(entry, entry.timestamp, entry.cfdfail);
    entry.cfdcorr = CalTime(entry);
    return entry;
}

bool CheckTimeGateAddback(const double &timediff)
{
   return timediff >= clover_addback_gate[0] && timediff <= clover_addback_gate[1];
}
