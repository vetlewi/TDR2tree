//
// Created by Vetle Wegner Ingeberg on 18/02/2021.
//

#include "TDREntry.h"
#include "TDRTypes.h"

#include <ostream>

using namespace TDR;

int64_t Entry_t::timestamp() const
{
  return (int64_t(info->info_field) << 28) | adc->timestamp;
}

unsigned Entry_t::Crate() const { return adc->crateID; }
unsigned Entry_t::Slot() const { return adc->slotID; }
unsigned Entry_t::Channel() const { return adc->chanID; }

struct addr_t {
    unsigned chanID : 4;
    bool tdc : 1;
    unsigned slotID : 3;
    unsigned crateID : 4;

    explicit operator uint16_t() const { return *reinterpret_cast<const uint16_t *>(this); }

    addr_t(const unsigned &chID, const bool &t, const unsigned &sID, const unsigned &cID){
        memset(this, 0, sizeof(addr_t));
        chanID = chID;
        tdc = t;
        slotID = sID;
        crateID = cID;
    }
};

uint16_t Entry_t::GetAddress() const
{
    return uint16_t(addr_t(adc->chanID, adc->tdc, adc->slotID, adc->crateID));
}

std::ostream &operator<<(std::ostream &os, const Entry_t &lhs)
{
  os << lhs.timestamp();
  return os;
}