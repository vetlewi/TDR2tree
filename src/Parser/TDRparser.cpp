//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#include <Parameters/experimentsetup.h>
#include <Parameters/Calibration.h>

#include "Parser/TDRparser.h"

#include <Buffer/Buffer.h>
#include <map>
#include <list>
#include <algorithm>

#if LOG_ENABLED
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#endif // LOG_ENABLED

using namespace Parser;


Entry_t MakeEntry(const TDR_entry &adc, const TDR_entry &tdc)
{
    assert( adc.address == tdc.address );
    assert( adc.timestamp == tdc.timestamp );


    Entry_t ret = {adc.address,
                   static_cast<uint16_t>(adc.evt->ADC_data),
                   static_cast<uint16_t>(tdc.evt->ADC_data),
                   adc.timestamp,
                   0,
                   0,
                   false,
                   false};
    return Calibrate(ret);
}

Entry_t MakeStandAloneEntry(const TDR_entry &adc)
{
    Entry_t ret = {adc.address,
                   ( adc.is_tdc ) ? static_cast<uint16_t>(0) : static_cast<uint16_t>(adc.evt->ADC_data),
                   ( adc.is_tdc ) ? static_cast<uint16_t>(adc.evt->ADC_data) : static_cast<uint16_t>(0),
                   adc.timestamp,
                   0,
                   0,
                   false,
                   false};
    return Calibrate(ret);
}

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

std::vector<Entry_t> TDRparser::SortMerge(std::vector<TDR_entry> &entries)
{
    std::vector<Entry_t> res;
    std::vector<TDR_explicit> leftover;
    for ( auto &entry : leftover_entries ){
        entries.emplace_back(TDR_entry(entry));
    }

    auto entry_pos = std::begin(entries);
    auto dres = std::back_inserter(res);
    bool merged;
    while ( entry_pos != std::end(entries) ){
        auto entry_pos_search = entry_pos + 1;
        merged = false;
        if ( (*entry_pos).is_merged ) {
            ++entry_pos;
            continue;
        }
        while ( entry_pos_search != std::end(entries) ){
            if ( *entry_pos == *entry_pos_search ){
                *dres++ = ( (*entry_pos).is_tdc ) ? MakeEntry(*entry_pos_search, *entry_pos) :
                        MakeEntry(*entry_pos, *entry_pos_search);
                (*entry_pos++).is_merged = true;
                (*entry_pos_search++).is_merged = true;
                merged = true;
                break;
            }
            ++entry_pos_search;
        }
        if ( !merged )
            ++entry_pos;
    }

    for ( auto &e : entries ){
        if ( !e.is_merged )
            leftover.emplace_back(e);
    }

    // We check if leftover entries follows from earlier buffers. If so, we add them to the output buffer
    // and set all the
    std::vector<TDR_explicit> keep;
    bool will_keep;
    for ( auto &l : leftover ){
        will_keep = true;
        for ( auto &l_old : leftover_entries ){
            if ( (l.address == l_old.address)&&(l.timestamp == l_old.timestamp)&&(l.is_tdc == l_old.is_tdc) ){
                will_keep = false;
#if LOG_ENABLED
                logger->info("Dropped entry:\n{}", l);
#endif // LOG_ENABLED
                *dres++ = MakeStandAloneEntry(TDR_entry(l));
            }
        }
        if ( will_keep )
            keep.push_back(l);
    }

    std::sort(std::begin(res), std::end(res), [](const Entry_t &lhs, const Entry_t &rhs){
        return ( double( lhs.timestamp - rhs.timestamp ) + ( lhs.cfdcorr  - rhs.cfdcorr ) ) < 0;});

    leftover_entries = keep;
    return res;
}


std::vector<Entry_t> TDRparser::GetEntry(const Fetcher::Buffer *new_buffer)
{
    size_t read = 0;
    const auto *buffer = reinterpret_cast<const Fetcher::TDRBuffer *>(new_buffer);
    const auto *raw_buffer = buffer->GetRawData();

    // First check if we have the first 'top timestamp'
    if ( top_time < 0 )
        top_time = FindTopTime(raw_buffer, buffer->GetSize());

    std::vector<TDR_entry> entries;
    const TDR_basic_type_t *entry;
    const TDR_event_type_t *evt_entry;
    while ( read < buffer->GetSize() ){
        entry = reinterpret_cast<const TDR_basic_type_t *>(raw_buffer + read);

        switch ( entry->ident ){
            case module_info :
                top_time = int64_t(reinterpret_cast<const TDR_info_type_t *>(raw_buffer + read)->info_field) << 28;
                break;
            case ADC_event :
                evt_entry = reinterpret_cast<const TDR_event_type_t *>(raw_buffer + read);
                entries.emplace_back(top_time+evt_entry->timestamp, evt_entry);
                break;
            default :
                break;
        }
        ++read;
    }
    return SortMerge(entries);
}
