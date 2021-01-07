//
// Created by Vetle Wegner Ingeberg on 01/10/2020.
//

#ifndef TDR2TREE_HDF5_WRITER_H
#define TDR2TREE_HDF5_WRITER_H

#include <hdf5.h>
#include <Parser/Entry.h>

#include <cstdint>

#define MAX_COLUMNS 512

struct Pointers {
    uint16_t *dtype;
    uint16_t *id;
    uint16_t *adcdata;
    uint16_t *cfddata;
    int64_t *timestamp;
    double *cfdcorr;
    double *energy;
    bool *cfdfail;
    bool *finishcode;

    void setup(char *map, haddr_t *offsets){
        dtype = (uint16_t *)(map + offsets[0]);
        id = (uint16_t *)(map + offsets[1]);
        adcdata = (uint16_t *)(map + offsets[2]);
        cfddata = (uint16_t *)(map + offsets[3]);
        timestamp = (int64_t *)(map + offsets[4]);
        cfdcorr = (double *)(map + offsets[5]);
        energy = (double *)(map + offsets[6]);
        cfdfail = (bool *)(map + offsets[7]);
        finishcode = (bool *)(map + offsets[8]);
    }

    void reset(){
        dtype = nullptr;
        id = nullptr;
        adcdata = nullptr;
        cfddata = nullptr;
        timestamp = nullptr;
        cfdcorr = nullptr;
        energy = nullptr;
        cfdfail = nullptr;
        finishcode = nullptr;
    }
};

class HDF5_Writer {

private:
    size_t noRows;
    size_t row_idx; // Current row

    int fd;
    Pointers points{};


public:

    HDF5_Writer() : noRows( 0 ), row_idx( 0 ), fd( 0 ){}

    HDF5_Writer(const char *fname, const int &nRows);

    ~HDF5_Writer();

    void allocate(const char *fname, const int &nRows);

    // Write an entry to file.
    bool Write(const Parser::Entry_t &entry);


};

#endif //TDR2TREE_HDF5_WRITER_H
