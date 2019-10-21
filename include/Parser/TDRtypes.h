//
// Created by Vetle Wegner Ingeberg on 21/10/2019.
//

#ifndef TDR2TREE_TDRTYPES_H
#define TDR2TREE_TDRTYPES_H

namespace Parser {
    struct tdr_data_header_t
    {
        char header_id[8];          /*!< Contains the string  EBYEDATA. (8 bytes) (byte 0-7) */
        uint32_t header_sequence;       /*!< Within the file. (4 bytes) (8-11) */
        uint16_t header_stream;        /*!< Data acquisition stream number (in the range 1=>4). (2 bytes) (12-13) */
        uint16_t header_tape;          /*!< =1. (2 bytes) (14-15) */
        uint16_t header_MyEndian;      /*!< Written as a native 1 by the tape server (2 bytes) (16-17) */
        uint16_t header_DataEndian;    /*!< Written as a native 1 in the hardware structure of the data following. (2 bytes) (18-19) */
        uint32_t header_dataLen;        /*!< Total length of useful data following the header in bytes. (4 bytes) (20-23) */
    };

    enum TDR_type
    {
        unknown = 0,
        sample_trace = 1,
        module_info = 2,
        ADC_event = 3
    };

    struct TDR_basic_type_t
    {
        unsigned unused_a
                : 32;
        unsigned unused_b
                : 30;
        TDR_type ident
                : 2;
    };

    struct TDR_event_type_t
    {
        unsigned timestamp : 28;
        unsigned unused : 4;
        unsigned ADC_data : 16;
        unsigned chanID : 12;
        bool veto : 1;
        bool fail : 1;
        TDR_type ident : 2;
    };

    struct TDR_info_type_t
    {
        unsigned timestamp
                : 28;
        unsigned unused
                : 2;
        unsigned info_field
                : 20;
        unsigned info_code
                : 4;
        unsigned module_number
                : 6;
        TDR_type ident
                : 2;
    };

    struct TDR_entry
    {
        int64_t timestamp;
        uint16_t address;
        bool is_tdc;
        const TDR_event_type_t *evt;

        TDR_entry()
                : timestamp(-1), address(0), is_tdc(false), evt(nullptr) {}


        TDR_entry(const int64_t &ts, const TDR_event_type_t *TDR)
                : timestamp(ts)
                , address((TDR->chanID & 0x10) ? TDR->chanID - 16 : TDR->chanID)
                , is_tdc((TDR->chanID & 0x10))
                , evt(TDR)
        {
            assert(evt != nullptr);
        }

        friend bool operator==(const TDR_entry &lhs, const TDR_entry &rhs);
    };

    bool operator==(const TDR_entry &lhs, const TDR_entry &rhs)
    {
        bool req = (lhs.address == rhs.address);
        req = req & (lhs.timestamp == rhs.timestamp);
        req = req & (lhs.is_tdc != rhs.is_tdc);
        return req;
    }


    struct TDR_leftover_entries
    {
        TDR_event_type_t data;
        TDR_entry entry;

        TDR_leftover_entries() = default;

        explicit TDR_leftover_entries(const TDR_entry &entr)
            : data( *entr.evt )
            , entry( entr.timestamp, &data )
        {}

    };

}

#endif //TDR2TREE_TDRTYPES_H
