//
// Created by Vetle Wegner Ingeberg on 18/02/2021.
//

#include "TDRParser.h"
#include "TDRTypes.h"

#include <future>
#include <algorithm>

using namespace TDR;

struct Address_t {
  unsigned chanID : 4;
  unsigned slotID : 3;
  unsigned crateID : 4;
};

struct Index_t
{
  std::vector<const TDR_event_type_t *> adc;
  std::vector<const TDR_event_type_t *> tdc;
  std::vector<const TDR_info_type_t *> info;
};

template<class T>
void parallel_sort(T* data, int len, int grainsize)
{
  // Use grainsize instead of thread count so that we don't e.g.
  // spawn 4 threads just to sort 8 elements.
  if(len < grainsize)
  {
    std::sort(data, data + len, std::less<T>());
  }
  else
  {
    auto future = std::async(parallel_sort<T>, data, len/2, grainsize);

    // No need to spawn another thread just to block the calling
    // thread which would do nothing.
    parallel_sort(data + len/2, len/2, grainsize);

    future.wait();

    std::inplace_merge(data, data + len/2, data + len, std::less<T>());
  }
}

std::vector<Entry_t> parallel_extract(std::vector<const char *> hdrs, const size_t &grainsize, bool start_at_zero = true)
{

  if ( hdrs.size() < grainsize ){
    Parser parser( ( start_at_zero ) ? nullptr : hdrs[0] );
    std::vector<Entry_t> result;
    for ( size_t idx = ( start_at_zero ) ? 0 : 1 ; idx < hdrs.size() ; ++idx ){
      auto r = parser.ParseBuffer(hdrs[idx], (idx == hdrs.size() - 1));
      result.insert(result.end(), r.begin(), r.end());
    }
    return result;
  }

  auto future = std::async(parallel_extract, std::vector<const char *>(hdrs.begin(), hdrs.begin()+hdrs.size()/2), grainsize, start_at_zero);
  auto r1 = parallel_extract(std::vector<const char *>(hdrs.begin()+hdrs.size()/2 - 1, hdrs.end()), grainsize, false);

  auto r0 = future.get();
  r0.insert(r0.end(), r1.begin(), r1.end());
  return r0;
}

std::vector<Entry_t> TDR::ParseFile(const char *begin, const char *end, const size_t &threads)
{
  // Find all headers
  std::vector<const char *> headers = FindHeaders(begin, end);

  // Next we will determine the number of buffers that each thread are suppose to read
  size_t grain_size = ( threads == 1 ) ? headers.size() : headers.size() / ( threads - 1 );
  grain_size = ( grain_size == 0 ) ? headers.size() : grain_size;

  auto entries = parallel_extract(headers, grain_size, true);

  grain_size = ( threads == 1 ) ? entries.size() : entries.size() / ( threads - 1 );
  grain_size = ( grain_size == 0 ) ? entries.size() : grain_size;
  parallel_sort(entries.data(), entries.size(), grain_size);

  return entries;
}

const char magic[] = {'E', 'B', 'Y', 'E', 'D', 'A', 'T', 'A'};
std::vector<const char *> TDR::FindHeaders(const char *begin, const char *end)
{
  std::vector<const char *> hdrs;
  auto *it = begin;
  while ( true ){
    it = std::search(it, end, magic, magic+sizeof(magic));
    if ( it >= end )
      break;
    hdrs.push_back(it);
    it += reinterpret_cast<const TDR_header_t *>(it)->header_dataLen + sizeof(TDR_header_t);
  }
  return hdrs;
}


Index_t Index_buffer(const char *buf)
{
  if ( !buf )
    return Index_t();

  auto *it = buf + sizeof(TDR_header_t);
  auto *end = buf + reinterpret_cast<const TDR_header_t *>(buf)->header_dataLen;

  const TDR_basic_type_t *entry;
  const TDR_info_type_t *info;
  const TDR_event_type_t *event;
  const TDR_trace_type_t *trace;

  Index_t index;

  while ( it < end ) {
    entry = reinterpret_cast<const TDR_basic_type_t *>(it);
    switch (entry->ident) {
      case unknown:  // If we cannot understand what the header is, we will move to next byte.
        ++it;
        break;
      case sample_trace:
        trace = reinterpret_cast<const TDR_trace_type_t *>(it);
        it += trace->size();
        break;
      case module_info:
        info = reinterpret_cast<const TDR_info_type_t *>(it);
        index.info.push_back(info);
        it += sizeof(TDR_info_type_t);
        break;
      case ADC_event:
        event = reinterpret_cast<const TDR_event_type_t *>(it);
        if (event->tdc)
          index.tdc.push_back(event);
        else
          index.adc.push_back(event);
        it += sizeof(TDR_event_type_t);
        break;
    }
  }
  return index;
}

void Parser::SetInfo(const TDR_info_type_t *info)
{
  for ( int channel = 0 ; channel < 16 ; ++channel ){
    channels[NUM_SLOTS*NUM_CHANNELS*info->crateID + NUM_CHANNELS*info->slotID + channel].AddInfo(info);
  }
}

void Parser::Initialize_info(const char *buf)
{
  if ( !buf )
    return;
  auto *it = buf + sizeof(TDR_header_t);
  auto *end = buf + reinterpret_cast<const TDR_header_t *>(buf)->header_dataLen;
  const TDR_basic_type_t *en;
  const TDR_info_type_t *inf;
  while ( it < end ){
    en = reinterpret_cast<const TDR_basic_type_t *>(it);
    if ( en->ident == TDR_type::module_info ){
      inf = reinterpret_cast<const TDR_info_type_t *>(it);
      SetInfo(inf);
    }
    it += sizeof(TDR_basic_type_t);
  }
}

void Parser::SetEntry(const TDR_event_type_t *event)
{
  if ( event->tdc )
    channels[NUM_SLOTS*NUM_CHANNELS*event->crateID + NUM_CHANNELS*event->slotID + event->chanID].AddTDC(event);
  else
    channels[NUM_SLOTS*NUM_CHANNELS*event->crateID + NUM_CHANNELS*event->slotID + event->chanID].AddADC(event);
}

void Parser::Parse_buffer(const char *buf)
{
  auto *it = buf + sizeof(TDR_header_t);
  auto *end = buf + reinterpret_cast<const TDR_header_t *>(buf)->header_dataLen;
  const TDR_basic_type_t *entry;
  const TDR_info_type_t *info;
  const TDR_event_type_t *event;
  const TDR_trace_type_t *trace;

  while ( it < end ){
    entry = reinterpret_cast<const TDR_basic_type_t *>(it);
    switch ( entry->ident ) {
      case unknown : // If we cannot understand what the header is, we will move to next byte.
        ++it;
        break;
      case sample_trace :
        trace = reinterpret_cast<const TDR_trace_type_t *>(it);
        it += trace->size();
        break;
      case module_info :
        info = reinterpret_cast<const TDR_info_type_t *>(it);
        SetInfo(info);
        it += sizeof(TDR_info_type_t);
        break;
      case ADC_event :
        event = reinterpret_cast<const TDR_event_type_t *>(it);
        SetEntry(event);
        it += sizeof(TDR_event_type_t);
        break;
    }
  }
}

std::vector<Entry_t> Parser::Collect(const char *buf)
{
  // We will collect all finished recreated events from previous buffer. Because there may be
  // information in the next buffer that are needed to extract information in this, we will
  // index all entries in the next buffer and find if there is anything useful in it.
  std::vector<Entry_t> entries;
  auto indexed = Index_buffer(buf);

  size_t count = 0;
  auto *address = reinterpret_cast<const Address_t *>(&count);
  for ( auto &channel : channels ){

    // Check that there isn't any events with missing 'partner'
    for ( auto &adc : channel.adc ){
      auto it = std::find(indexed.tdc.begin(), indexed.tdc.end(), *adc);
      if ( it != indexed.tdc.end() )
        channel.AddTDC(*it);
    }

    for ( auto &tdc : channel.tdc ){
      auto it = std::find(indexed.adc.begin(), indexed.adc.end(), *tdc);
      if ( it != indexed.adc.end() )
        channel.AddADC(*it);
    }

    // If the buffer of the channel isn't empty it might mean that
    // there is a info entry missing, preventing the event from being fully
    // reconstructed.
    if ( !channel.buffer.empty() ){
      auto it = std::find_if(indexed.info.begin(), indexed.info.end(),
                             [&address](const TDR_info_type_t *inf)
                             { return ( address->crateID == inf->crateID ) && (address->slotID == inf->slotID); });
      if ( it != indexed.info.end() )
        channel.AddInfo(*it);
    }
    entries.insert(entries.end(), channel.events.begin(), channel.events.end());
    channel.clear(true);
    ++count;
  }
  std::sort(entries.begin(), entries.end(), [](const Entry_t &lhs, const Entry_t &rhs){
    return lhs.timestamp() < rhs.timestamp();
  });
  return entries;
}

std::vector<Entry_t> Parser::ParseBuffer(const char *buf, const bool &eof)
{
  auto entries = Collect(buf);
  Parse_buffer(buf);

  if ( eof ){
    auto more_entries = Collect(nullptr);
    entries.insert(entries.end(), more_entries.begin(), more_entries.end());
    std::sort(entries.begin(), entries.end(), [](const Entry_t &lhs, const Entry_t &rhs){
      return lhs.timestamp() < rhs.timestamp();
    });
  }
  return entries;
}