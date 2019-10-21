//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#include <Parameters/experimentsetup.h>
#include <Parameters/Calibration.h>

#include "Parser/TDRparser.h"

#include <Buffer/Buffer.h>
#include <map>

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
    std::vector<TDR_leftover_entries> leftover;
    for ( auto &entry : leftover_entries ){
        entries.push_back(entry.entry);
    }


    // First sort the entries by the timestamp
    std::sort(entries.begin(), entries.end(), [](const TDR_entry &lhs, const TDR_entry &rhs){ return lhs.timestamp < rhs.timestamp; });

    std::vector<TDR_entry> v, vnew;
    bool found;
    v = entries;

    while ( v.size() > 0 ){
        found = false;
        vnew.clear();
        for ( size_t i = 1 ; i < v.size() ; ++i){
            if ( v[0] == v[i] ){
                found = true;
                res.push_back(( v[0].is_tdc ) ?  MakeEntry(v[i], v[0]) : MakeEntry(v[0], v[i]));
                vnew.insert(vnew.end(), v.begin()+i+1, v.end());
                break;
            } else {
                vnew.push_back(v[i]);
            }
        }
        if ( !found ){
            leftover.emplace_back(v[0]);
        }
        v = vnew;
    }
    leftover_entries = leftover;
    return res;
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