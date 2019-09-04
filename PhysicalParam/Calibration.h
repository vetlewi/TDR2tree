#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "BasicStruct.h"

extern bool SetCalibration(const char *calfile);

extern word_t &CalibrateCFD(word_t &detector);

extern double CalibrateEnergy(word_t &detector);

extern double CalibrateTime(word_t &detector);

extern word_t &Calibrate(word_t &evt);

extern bool CheckTimeGateAddback(const double &timediff);


#endif // CALIBRATION_H
