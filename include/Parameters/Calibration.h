#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <Parser/Entry.h>

bool SetCalibration(const char *calfile);

//Parser::Entry_t &CalibrateCFD(Parser::Entry_t &detector);

//Parser::Entry_t &CalibrateEnergy(Parser::Entry_t &detector);

//Parser::Entry_t &CalibrateTime(Parser::Entry_t &detector);

Parser::Entry_t &Calibrate(Parser::Entry_t &evt);

bool CheckTimeGateAddback(const double &timediff);


#endif // CALIBRATION_H
