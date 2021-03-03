//
// Created by Vetle Wegner Ingeberg on 25/01/2021.
//

#ifndef TDRTYPES_H
#define TDRTYPES_H

#include <cstdint>
#include <iosfwd>

namespace TDR {

  struct TDR_header_t {
    char header_id[8];        /*!< Contains the string  EBYEDATA. (8 bytes) (byte 0-7) */
    uint32_t header_sequence; /*!< Within the file. (4 bytes) (8-11) */
    uint16_t
        header_stream; /*!< Data acquisition stream number (in the range 1=>4). (2 bytes) (12-13) */
    uint16_t header_tape;       /*!< =1. (2 bytes) (14-15) */
    uint16_t header_MyEndian;   /*!< Written as a native 1 by the tape server (2 bytes) (16-17) */
    uint16_t header_DataEndian; /*!< Written as a native 1 in the hardware structure of the data
                                   following. (2 bytes) (18-19) */
    uint32_t header_dataLen;    /*!< Total length of useful data following the header in bytes. (4
                                   bytes)    (20-23) */
  };

  enum TDR_type { unknown = 0, sample_trace = 1, module_info = 2, ADC_event = 3 };

  struct TDR_basic_type_t {
    unsigned unused_a : 32;
    unsigned unused_b : 30;
    TDR_type ident : 2;
  };

  struct TDR_event_type_t {
    unsigned timestamp : 28;
    unsigned unused : 4;
    unsigned ADC_data : 16;

    unsigned chanID : 4;
    bool tdc : 1;
    unsigned slotID : 3;
    unsigned crateID : 4;
    bool veto : 1;
    bool fail : 1;
    TDR_type ident : 2;

  };

  struct TDR_info_type_t {
    unsigned timestamp : 28;
    unsigned unused : 2;
    unsigned info_field : 20;
    unsigned info_code : 4;
    unsigned slotID : 3;
    unsigned crateID : 3;
    TDR_type ident : 2;
  };

  struct TDR_trace_type_t {
    unsigned timestamp : 28;
    unsigned unused1 : 4;
    unsigned sample_length : 16;

    unsigned chanID : 4;
    bool tdc : 1;
    unsigned slotID : 3;
    unsigned crateID : 4;
    unsigned unused0 : 2;
    TDR_type ident : 2;

    // Get the number of following bytes that are traces
    [[nodiscard]] inline size_t size() const { return sample_length; }
  };

  unsigned CrateID(const TDR_basic_type_t *evt);
  unsigned SlotID(const TDR_basic_type_t *evt);

  bool operator==(const TDR::TDR_event_type_t &lhs, const TDR::TDR_event_type_t &rhs);
  inline bool operator==(const TDR::TDR_event_type_t *lhs, const TDR::TDR_event_type_t &rhs) {
    return (*lhs) == rhs;
  }

  std::ostream &operator<<(std::ostream &os, const TDR::TDR_header_t &hdr);
  std::ostream &operator<<(std::ostream &os, const enum TDR::TDR_type &type);
  std::ostream &operator<<(std::ostream &os, const TDR::TDR_event_type_t &hdr);
  std::ostream &operator<<(std::ostream &os, const TDR::TDR_info_type_t &hdr);
}

#endif  // TDRTYPES_H
