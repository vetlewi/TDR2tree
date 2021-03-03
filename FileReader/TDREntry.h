//
// Created by Vetle Wegner Ingeberg on 18/02/2021.
//

#ifndef TDRENTRY_H
#define TDRENTRY_H

#include <iosfwd>

#include <cstdint>

namespace TDR {

  struct TDR_event_type_t;
  struct TDR_info_type_t;

  class UserSetup;

  struct Entry_t
  {
    //! ADC entry
    const TDR_event_type_t *adc;

    //! TDC entry
    const TDR_event_type_t *tdc;

    //! Info entry
    const TDR_info_type_t *info;

    //! Get the timestamp
    [[nodiscard]] int64_t timestamp() const;

    //! Get the crate
    [[nodiscard]] unsigned Crate() const;
    [[nodiscard]] unsigned Slot() const;
    [[nodiscard]] unsigned Channel() const;

    uint16_t GetAddress() const;
  };

  inline bool operator<(const Entry_t &lhs, const Entry_t &rhs) {
    return lhs.timestamp() < rhs.timestamp();
  }

  inline bool operator<=(const Entry_t &lhs, const Entry_t &rhs){
    return lhs.timestamp() <= rhs.timestamp();
  }

  inline bool operator>(const Entry_t &lhs, const Entry_t &rhs) {
    return lhs.timestamp() > rhs.timestamp();
  }

  inline int64_t operator-(const Entry_t &lhs, const Entry_t &rhs){
    return lhs.timestamp() - rhs.timestamp();
  }

  std::ostream &operator<<(std::ostream &os, const Entry_t &lhs);

}


#endif  // TDRENTRY_H
