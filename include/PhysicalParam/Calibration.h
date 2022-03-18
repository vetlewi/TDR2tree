#ifndef CALIBRATION_H
#define CALIBRATION_H

struct word_t;

extern bool SetCalibration(const char *calfile);

extern bool SetRangeFile(const char *rangefile);

extern double GetRange(const double &energy);

extern double CalibrateEnergy(const word_t &detector);

//! Calibrate the time (± a few ns)
extern double CalibrateTime(const word_t &detector);

//! Align the time with a contant integer
extern int CalibrateCoarseTime(const word_t &detector);

extern bool CheckTimeGateAddback(const double &timediff);


#endif // CALIBRATION_H
