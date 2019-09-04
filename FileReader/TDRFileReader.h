//
// Created by Vetle Wegner Ingeberg on 02/09/2019.
//

#ifndef TDRFILEREADER_H
#define TDRFILEREADER_H

#include "FileReader.h"
#include "BufferType.h"

#include <vector>
#include <cstdint>

#include "BasicStruct.h"

#define TDRBUFFER_SIZE 65536

class TDRFileReader : public FileReader
{

public:

    //! Initializer.
    TDRFileReader() : FileReader(new TDRBuffer ), top_time( -1 ), p_buffer{}{}

    //! Read a buffer.
    int Read(Buffer &buffer) override;

private:

    //! Top 20-bits of the timestamp.
    int top_time;

    //! Buffer to keep the raw data in memory.
    char p_buffer[TDRBUFFER_SIZE];

    //! Find the top 20-bit timestamp.
    void FindTopTime(const uint64_t *data,  /*!< Pointer to raw data    */
                     const int &size        /*!< Size of teh raw data   */);

    //! Process the raw data.
    std::vector<word_t> Process_data(const uint64_t *data,  /*!< Pointer to raw data    */
                                     const int &size        /*!< Size of teh raw data   */);

    //! Calibrate timestamps and energy of the events.
    std::vector<word_t> &Calibrate_data(std::vector<word_t> &data);

};


#endif //TDR2TREE_TDRFILEREADER_H
