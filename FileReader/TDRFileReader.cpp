//
// Created by Vetle Wegner Ingeberg on 02/09/2019.
//

#include "TDRFileReader.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>

#include "BasicStruct.h"
#include "Calibration.h"
#include "experimentsetup.h"
#include "BufferType.h"

struct DATA_HEADER_T {
    char     header_id[8];          /*!< Contains the string  EBYEDATA. (8 bytes) (byte 0-7) */
    uint32_t header_sequence;       /*!< Within the file. (4 bytes) (8-11) */
    uint16_t  header_stream;        /*!< Data acquisition stream number (in the range 1=>4). (2 bytes) (12-13) */
    uint16_t  header_tape;          /*!< =1. (2 bytes) (14-15) */
    uint16_t  header_MyEndian;      /*!< Written as a native 1 by the tape server (2 bytes) (16-17) */
    uint16_t  header_DataEndian;    /*!< Written as a native 1 in the hardware structure of the data following. (2 bytes) (18-19) */
    uint32_t header_dataLen;        /*!< Total length of useful data following the header in bytes. (4 bytes) (20-23) */
};


struct TDRbasic {
    uint64_t unused : 62;
    unsigned iden : 2;
};

struct TDRinfo {
    unsigned timestamp : 28;
    unsigned unused : 4;
    unsigned info_field : 20;
    unsigned info_code : 4;
    unsigned slotID : 3;
    unsigned crateID : 3;
    unsigned iden : 2;
};

struct TDRdata {
    unsigned timestamp : 28;
    unsigned unused : 4;
    unsigned adcdata : 16;
    unsigned chanID : 4;
    unsigned isCFD : 1;
    unsigned slotID : 3;
    unsigned crateID : 4;
    unsigned veto : 1;
    unsigned fail : 1;
    unsigned iden : 2;
};

struct TDRdataFTS {
    int64_t timestamp;
    unsigned adcdata : 16;
    unsigned chanID : 4;
    unsigned isCFD : 1;
    unsigned slotID : 3;
    unsigned crateID : 4;
    unsigned veto : 1;
    unsigned fail : 1;
    unsigned iden : 2;
};

bool operator==(const TDRdata &lhs, const TDRdata &rhs)
{
    return ( lhs.timestamp == rhs.timestamp ) && ( lhs.chanID == rhs.chanID ) && (lhs.isCFD != rhs.isCFD )
    && ( lhs.slotID == rhs.slotID ) && ( lhs.crateID == rhs.crateID );
}

static TDRdata glob_tmp;


int CountDataW(const uint64_t *data, const int &size){
    int read = 0, found = 0;
    while ( read < size ){
        if ( reinterpret_cast<const TDRbasic *>(data+read++)->iden == 3 )
            ++found;
    }
    return found;
}

// #########################################################


int TDRFileReader::Read(Buffer &buffer)
{
    // First we will read a new block of data from file.
    int raw_length = ReadRaw(p_buffer, TDRBUFFER_SIZE);
    const DATA_HEADER_T *buffer_header = reinterpret_cast<const DATA_HEADER_T *>(p_buffer);
    const uint64_t *data = reinterpret_cast<const uint64_t *>(p_buffer + sizeof(DATA_HEADER_T));
    const int bufsize = buffer_header->header_dataLen / sizeof( uint64_t );

    if ( top_time <= 0 )
        FindTopTime(data, bufsize);

    std::vector<word_t> events = Process_data(data, bufsize);
    events = Calibrate_data(events);

    // We will now copy to the buffer. However, we need to know that we have enough space!
    if ( buffer.GetSize() != events.size() )
        buffer.Resize(events.size());

    memcpy(buffer.GetBuffer(), reinterpret_cast<char*>(events.data()), sizeof(word_t)*events.size());

    if ( raw_length == TDRBUFFER_SIZE )
        return true;

    return ( IsError() ) ? -1 : 0;
}


void TDRFileReader::FindTopTime(const uint64_t *data, const int &size)
{
    int read = 0;
    while ( read < size ){
        if ( reinterpret_cast<const TDRbasic *>(data+read)->iden == unsigned(2) ){
            top_time = int(reinterpret_cast<const TDRinfo *>(data+read)->info_field);
            break;
        }
        ++read;
    }
}

std::vector<word_t> TDRFileReader::Process_data(const uint64_t *data, const int &size)
{
    std::vector<word_t> events;
    int read = 0;
    const TDRdata *adc=nullptr, *cfd=nullptr, *tmp;
    const TDRbasic *basic;

    if ( glob_tmp.iden == 3 ){
        if ( glob_tmp.isCFD )
            cfd = &glob_tmp;
        else
            adc = &glob_tmp;
    }

    int size_of_data = CountDataW(data,size);
    TDRdataFTS *dat = new TDRdataFTS[size_of_data];
    int set = 0;
    while ( read < size ){

        basic = reinterpret_cast<const TDRbasic *>(data + read);

        if ( basic->iden == unsigned(2) ){
            top_time = int(reinterpret_cast<const TDRinfo *>(data+read)->info_field);
        } else if ( basic->iden == unsigned(3) ) {
            tmp = reinterpret_cast<const TDRdata *>(data+read);
            dat[set++] = {(top_time<<20)|tmp->timestamp,tmp->adcdata,tmp->chanID,tmp->isCFD,tmp->slotID,
                          tmp->crateID,tmp->veto,tmp->fail,tmp->iden};
            if ( tmp->isCFD )
                cfd = tmp;
            else
                adc = tmp;
        } else {
            std::cerr << __PRETTY_FUNCTION__ << ": Error, wrong identifier. Got " << basic->iden << std::endl;
        }

        // Check if we have both a cfd and a adc word.
        if ( adc && cfd ){
            if ( *adc == *cfd ){
                events.push_back({uint16_t(adc->crateID), uint16_t(adc->slotID), uint16_t(adc->chanID),
                                  uint16_t(adc->adcdata), uint16_t(cfd->adcdata),0, 0,
                                  (unsigned(uint64_t(top_time) >> unsigned(20))|adc->timestamp), 0, 0});
            }
            adc = nullptr;
            cfd = nullptr;
        }
        ++read;
    }

    if ( adc )
        glob_tmp = *adc;
    else if ( cfd )
        glob_tmp = *cfd;
    else
        glob_tmp.iden = 0;

    return events;
}

std::vector<word_t> &TDRFileReader::Calibrate_data(std::vector<word_t> &data)
{
    for ( auto &event : data){
        event = Calibrate(event);
    }
    std::sort(std::begin(data), std::end(data), [](const word_t &lhs, const word_t &rhs){
        return double( lhs.timestamp  - rhs.timestamp ) + ( lhs.cfdcorr - rhs.cfdcorr ) < 0;
    });
    return data;
}