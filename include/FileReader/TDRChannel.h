//
// Created by Vetle Wegner Ingeberg on 18/02/2021.
//

#ifndef TDRCHANNEL_H
#define TDRCHANNEL_H

#include <vector>

#include <TDREntry.h>

namespace TDR
{
  struct TDR_basic_type_t;
  struct TDR_event_type_t;
  struct TDR_info_type_t;

  struct Channel_t {

    std::vector<const TDR_event_type_t *> adc;
    std::vector<const TDR_event_type_t *> tdc;
    const TDR_info_type_t *info{nullptr};

    std::vector<Entry_t> buffer;
    std::vector<Entry_t> events;

    void FlushBuffer(const TDR_info_type_t *inf);
    void PushEntry(const Entry_t &&entry);
    void AddADC(const TDR_event_type_t *evt);
    void AddTDC(const TDR_event_type_t *evt);
    void AddInfo(const TDR_info_type_t *evt);
    void AddEntry(const TDR_basic_type_t *evt);

    void clear(const bool &keep_info = false);

    std::vector<Entry_t> GetEntries();

    Channel_t() : info(nullptr) {}

  };

}

#endif  // TDRCHANNEL_H
