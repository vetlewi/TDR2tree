//
// Created by Vetle Wegner Ingeberg on 18/02/2021.
//

#include "TDRTypes.h"

#include <ostream>

using namespace TDR;

unsigned CrateID(const TDR_basic_type_t *evt) {
  if (evt->ident == module_info)
    return reinterpret_cast<const TDR_info_type_t *>(evt)->crateID;
  else if (evt->ident == ADC_event)
    return reinterpret_cast<const TDR_event_type_t *>(evt)->crateID;
  else
    return -1;
}

unsigned SlotID(const TDR_basic_type_t *evt) {
  if (evt->ident == module_info)
    return reinterpret_cast<const TDR_info_type_t *>(evt)->slotID;
  else if (evt->ident == ADC_event)
    return reinterpret_cast<const TDR_event_type_t *>(evt)->slotID;
  else
    return -1;
}

bool TDR::operator==(const TDR_event_type_t &lhs, const TDR_event_type_t &rhs) {
  return (lhs.crateID == rhs.crateID) && (lhs.slotID == rhs.slotID) && (lhs.chanID == rhs.chanID)
         && (lhs.timestamp == rhs.timestamp) && (lhs.tdc != rhs.tdc);
}

std::ostream &TDR::operator<<(std::ostream &os, const TDR_header_t &hdr) {
  char tmp[9];
  for (int i = 0; i < 8; ++i) {
    tmp[i] = hdr.header_id[i];
  }
  tmp[8] = '\0';

  os << "\tHeader id: " << tmp << '\n';
  os << "\tHeader sequence: " << hdr.header_sequence << '\n';
  os << "\tHeader stream: " << hdr.header_stream << '\n';
  os << "\tHeader tape: " << hdr.header_tape << '\n';
  os << "\tHeader MyEndian: " << hdr.header_MyEndian << '\n';
  os << "\tHeader DataEndian: " << hdr.header_DataEndian << '\n';
  os << "\tHeader Data length: " << hdr.header_dataLen << '\n';

  return os;
}


std::ostream &TDR::operator<<(std::ostream &os, const TDR_event_type_t &hdr) {
  os << "\tIdentity: " << hdr.ident << "\n";
  os << "\tFail: " << std::boolalpha << hdr.fail << "\n";
  os << "\tVeto: " << std::boolalpha << hdr.veto << "\n";
  os << "\tCrate #: " << hdr.crateID << "\n";
  os << "\tSlot #: " << hdr.slotID << "\n";
  os << "\tTDC flag: " << std::boolalpha << hdr.tdc << "\n";
  os << "\tChannel #: " << hdr.chanID << "\n";
  os << "\tADC data: " << hdr.ADC_data << "\n";
  os << "\tTimestamp: " << hdr.timestamp << "\n";
  return os;
}


std::ostream &TDR::operator<<(std::ostream &os, const TDR_info_type_t &hdr) {
  os << "\tIdentity: " << hdr.ident << "\n";
  os << "\tCrate #: " << hdr.crateID << "\n";
  os << "\tSlot #: " << hdr.slotID << "\n";
  os << "\tInfo code: " << hdr.info_code << "\n";
  os << "\tInfo field: " << hdr.info_field << "\n";
  os << "\tTimestamp: " << hdr.timestamp << "\n";
  return os;
}

std::ostream &TDR::operator<<(std::ostream &os, const enum TDR_type &type) {
  switch (type) {
    case unknown:
      os << "unknown";
      break;
    case sample_trace:
      os << "sample_trace";
      break;
    case module_info:
      os << "module_info";
      break;
    case ADC_event:
      os << "ADC_event";
      break;
  }
  return os;
}