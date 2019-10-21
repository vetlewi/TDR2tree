//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#include <Parameters/experimentsetup.h>

#include "Parser/TDRparser.h"

#include <Buffer/Buffer.h>

using namespace Parser;


struct data_header_t {
    char     header_id[8];          /*!< Contains the string  EBYEDATA. (8 bytes) (byte 0-7) */
    uint32_t header_sequence;       /*!< Within the file. (4 bytes) (8-11) */
    uint16_t  header_stream;        /*!< Data acquisition stream number (in the range 1=>4). (2 bytes) (12-13) */
    uint16_t  header_tape;          /*!< =1. (2 bytes) (14-15) */
    uint16_t  header_MyEndian;      /*!< Written as a native 1 by the tape server (2 bytes) (16-17) */
    uint16_t  header_DataEndian;    /*!< Written as a native 1 in the hardware structure of the data following. (2 bytes) (18-19) */
    uint32_t header_dataLen;        /*!< Total length of useful data following the header in bytes. (4 bytes) (20-23) */
};

enum TDR_type {
    unknown = 0,
    sample_trace = 1,
    module_info = 2,
    ADC_event = 3
};

struct TDR_basic_type_t {
    unsigned unused_a : 32;
    unsigned unused_b : 30;
    TDR_type ident : 2;
};

struct TDR_event_type_t {
    unsigned timestamp : 28;
    unsigned unused : 4;
    unsigned ADC_data : 16;
    unsigned chanID : 12;
    bool veto : 1;
    bool fail : 1;
    TDR_type ident : 2;
};

struct TDR_info_type_t {
    unsigned timestamp : 28;
    unsigned unused : 2;
    unsigned info_field : 20;
    unsigned info_code : 4;
    unsigned module_number : 6;
    TDR_type ident : 2;
};

struct TDR_entry {
    int64_t timestamp;
    uint16_t address;
    const TDR_event_type_t *adc;
    const TDR_event_type_t *tdc;

    TDR_entry(const int64_t &ts, const uint16_t &adr, const TDR_event_type_t *evt)
        : timestamp( ts )
        , address( ( evt->chanID & 0x10 ) ? evt->chanID-16 : evt->chanID )
        , adc( nullptr )
        , tdc( nullptr )
    {
        adc = ( evt->chanID & 0x10 ) ? nullptr : evt;
        tdc = ( evt->chanID & 0x10 ) ? evt : nullptr;
    }

    TDR_entry(const int64_t &ts, const uint16_t &adr, const TDR_event_type_t *ADC, const TDR_event_type_t *TDC)
            : timestamp( ts )
            , address( ADC->chanID )
            , adc( ADC )
            , tdc( TDC )
    {
        assert((adc->chanID & 0xF) == (tdc->chanID & 0xF))
    }

};


int64_t FindTopTime(const uint64_t *raw, const size_t &size)
{
    size_t read = 0;
    const TDR_basic_type_t *entry;
    while ( read < size ){
        entry = reinterpret_cast<const TDR_basic_type_t *>(raw + read);
        if ( entry->ident == TDR_type::module_info ){
            return int64_t(reinterpret_cast<const TDR_info_type_t *>(raw + read)->info_field) << 28;
        }
        ++read;
    }
    return -1;
}


std::vector<Entry_t> TDRparser::GetEntry(const Fetcher::Buffer *new_buffer, Status &status)
{
    size_t read = 0;
    const auto *buffer = reinterpret_cast<const Fetcher::TDRBuffer *>(new_buffer);
    const auto *raw_buffer = buffer->GetRawData();

    // First check if we have the first 'top timestamp'
    if ( top_time < 0 )
        top_time = FindTopTime(raw_buffer, buffer->GetSize());

    std::vector<TDR_entry> entries;
    const TDR_basic_type_t *entry;
    while ( read < buffer->GetSize() ){
        entry = reinterpret_cast<const TDR_basic_type_t *>(raw_buffer + read);

        switch ( entry->ident ){
            case module_info :
                top_time = int64_t(reinterpret_cast<const TDR_info_type_t *>(raw_buffer + read)->info_field) << 28;
                break;
            case ADC_event :
                entries.emplace_back(top_time+reinterpret_cast<const TDR_event_type_t *>(raw_buffer + read)->timestamp,
                                     reinterpret_cast<const TDR_event_type_t *>(raw_buffer + read));
                break;
            default :
                break;
        }
        ++read;
    }

    // Next we need to sort the entries by time:
    std::sort(entries.begin(), entries.end(), [](const TDR_entry &lhs, const TDR_entry &rhs){ lhs.timestamp < rhs.timestamp; });

    // Next we will make a vector for each of the channels.


}