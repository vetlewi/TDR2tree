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

Parser::Entry_t &CalibrateEnergy(Parser::Entry_t &detector)
{
    DetectorInfo_t dinfo = GetDetector(detector.address);
    switch (dinfo.type) {
        case labr_3x8 :
            detector.energy = gain_labrL[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_labrL[dinfo.detectorNum];
            break;
        case labr_2x2_ss :
            detector.energy = gain_labrS[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_labrS[dinfo.detectorNum];
            break;
        case labr_2x2_fs :
            detector.energy = gain_labrF[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_labrF[dinfo.detectorNum];
            break;
        case clover :
            detector.energy = gain_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS+dinfo.telNum]*(detector.adcdata + drand48() - 0.5) + shift_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS+dinfo.telNum];
            break;
        case de_ring :
            detector.energy = gain_ring[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_ring[dinfo.detectorNum];
            break;
        case de_sect :
            detector.energy = gain_sect[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_sect[dinfo.detectorNum];
            break;
        case eDet :
            detector.energy = gain_back[dinfo.detectorNum]*(detector.adcdata + drand48() - 0.5) + shift_back[dinfo.detectorNum];
            break;
        default :
            detector.energy = detector.adcdata;
            break;
    }
    return detector;
}

Parser::Entry_t &CalibrateCFD(Parser::Entry_t &detector)
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
}

Parser::Entry_t &CalibrateTime(Parser::Entry_t &detector)
{
    DetectorInfo_t dinfo = GetDetector(detector.address);
    switch (dinfo.type) {
        case labr_3x8 :
            detector.cfdcorr = detector.cfdcorr + shift_t_labrL[dinfo.detectorNum];
            break;
        case labr_2x2_ss :
            detector.cfdcorr = detector.cfdcorr + shift_t_labrS[dinfo.detectorNum];
            break;
        case labr_2x2_fs :
            detector.cfdcorr = detector.cfdcorr + shift_t_labrF[dinfo.detectorNum];
            break;
        case clover :
            detector.cfdcorr = detector.cfdcorr + shift_t_clover[dinfo.detectorNum*NUM_CLOVER_CRYSTALS + dinfo.telNum];
            break;
        case de_ring :
            detector.cfdcorr = detector.cfdcorr + shift_t_ring[dinfo.detectorNum];
            break;
        case de_sect :
            detector.cfdcorr = detector.cfdcorr + shift_t_sect[dinfo.detectorNum];
            break;
        case eDet :
            detector.cfdcorr = detector.cfdcorr + shift_t_back[dinfo.detectorNum];
            break;
        default :
            detector.cfdcorr = detector.cfdcorr;
            break;
    }
    return detector;
}

Parser::Entry_t &CalibrateEntry(Parser::Entry_t &entry)
{
    entry = CalibrateEnergy(entry);
    entry = CalibrateCFD(entry);
    entry = CalibrateTime(entry);
    return entry;
}

bool CheckTimeGateAddback(const double &timediff)
{
   return timediff >= clover_addback_gate[0] && timediff <= clover_addback_gate[1];
}
