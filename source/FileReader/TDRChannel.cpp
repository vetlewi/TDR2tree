//
// Created by Vetle Wegner Ingeberg on 18/02/2021.
//

#include "TDRChannel.h"
#include "TDRTypes.h"

using namespace TDR;

void Channel_t::FlushBuffer(const TDR_info_type_t *inf)
{
  for ( auto &entry : buffer ){
    entry.info = inf;
    if ( events.empty() )
      events.push_back(entry);
    else if ( (events.back() <= entry) )
      events.push_back(entry);
    //else
    //  throw std::runtime_error("Timestamp missmatch");
  }
  buffer.clear();
}

void Channel_t::PushEntry(const Entry_t &&entry)
{
  if ( entry.info ){
    if ( events.empty() ) {
      events.push_back(entry);
    } else if ( (events.back() <= entry) ) {
      events.push_back(entry);
    } else {
      info = nullptr;
      buffer.push_back(entry);
      buffer.back().info = nullptr;
    }
  } else {
    buffer.push_back(entry);
  }
}

void Channel_t::AddADC(const TDR_event_type_t *evt) {
  auto it = std::find(tdc.begin(), tdc.end(), *evt);
  if (it != tdc.end()) {
    //events.push_back({evt, *it, info});
    PushEntry({evt, *it, info});
    //tdc.erase(it);
  } else {
    adc.push_back(evt);
  }
}

void Channel_t::AddTDC(const TDR_event_type_t *evt) {
  auto it = std::find(adc.begin(), adc.end(), *evt);
  if (it != adc.end()) {
    //events.push_back({*it, evt, info});
    PushEntry({*it, evt, info});
    adc.erase(it);
  } else {
    tdc.push_back(evt);
  }
}

void Channel_t::AddInfo(const TDR_info_type_t *inf) {
  FlushBuffer(inf);
  info = inf;
}

void Channel_t::AddEntry(const TDR_basic_type_t *evt) {
  if (evt->ident == TDR_type::ADC_event) {
    auto *ev = reinterpret_cast<const TDR_event_type_t *>(evt);
    if (ev->tdc)
      AddTDC(ev);
    else
      AddADC(ev);
  } else if (evt->ident == TDR_type::module_info) {
    AddInfo(reinterpret_cast<const TDR_info_type_t *>(evt));
  } else {
    throw std::runtime_error("Unknown event type.");
  }
}

void Channel_t::clear(const bool &keep_info)
{
  adc.clear();
  tdc.clear();
  buffer.clear();
  events.clear();
  if (!keep_info)
    info = nullptr;
}

std::vector<Entry_t> Channel_t::GetEntries()
{

  if ( !info )
    return std::vector<Entry_t>();

  // Check if everything is sorted
  auto it = std::is_sorted_until(events.begin(), events.end());
  std::vector<Entry_t> data(events.begin(), it);
  if ( it == events.end() ){
    data.clear();
  } else {
    events.erase(events.begin(), it);
    info = nullptr;
  }
  return data;
}