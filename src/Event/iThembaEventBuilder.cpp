//
// Created by Vetle Wegner Ingeberg on 31/10/2019.
//

#include "Event/iThembaEventBuilder.h"

#include <Parameters/experimentsetup.h>

#include <cmath>

using namespace Event;

#define MIN_NUM_BUFFERS 0x2000

std::vector<iThembaEvent> BuildEvents(std::vector<Parser::Entry_t> &entries)
{
    std::vector<iThembaEvent> result;
    std::vector<Parser::Entry_t> event;

    auto entry = std::begin(entries);

    while ( entry != std::end(entries) ){

        if ( GetDetectorType((*entry).address) == DetectorType::eDet ){
            auto search_entry = --entry;
            event.clear();
            auto devent = std::back_inserter(event);
            *devent++ = *entry;
            while ( search_entry >= std::begin(entries) ){
                if ( fabs( ( double((*search_entry).timestamp - (*entry).timestamp) + ((*search_entry).cfdcorr - (*entry).cfdcorr)) ) < 1500 ){
                    *devent++ = *search_entry--;
                } else {
                    break;
                }
            }

            search_entry = entry;
            while ( search_entry != std::end(entries) ){
                if ( fabs( ( double((*search_entry).timestamp - (*entry).timestamp) + ((*search_entry).cfdcorr - (*entry).cfdcorr)) ) < 1500 ){
                    *devent++ = *search_entry++;
                } else {
                    break;
                }
            }
            result.emplace_back(event);
        }

    }
    return result;
}

inline double TimeDiff(const Parser::Entry_t &lhs, const Parser::Entry_t &rhs)
{
    return double(lhs.timestamp - rhs.timestamp) + (lhs.cfdcorr - rhs.cfdcorr);
}

std::vector<iThembaEvent> iThembaEventBuilder::BuildEvents(std::vector<Parser::Entry_t> &entries)
{
    // First we fill the buffer
    entry_buffer.insert(entry_buffer.begin(), std::begin(entries), std::end(entries));

    // Sort the entry buffer
    /*std::sort(std::begin(entry_buffer), std::end(entry_buffer),
              [](const Parser::Entry_t &lhs, const Parser::Entry_t &rhs) {
                  return (double(lhs.timestamp - rhs.timestamp) + (lhs.cfdcorr - rhs.cfdcorr)) < 0;
              });*/

    // Check if we have enough enough events, if not return empty.
    if (entry_buffer.size() < MIN_NUM_BUFFERS) {
        return std::vector<iThembaEvent>();
    }
    std::vector<iThembaEvent> events;
    std::vector<Parser::Entry_t> event;
    auto current_entry = std::begin(entry_buffer);
    auto first_keep = std::end(entry_buffer) - MIN_NUM_BUFFERS;
    while (current_entry != std::end(entry_buffer) - MIN_NUM_BUFFERS) {
        if (GetDetectorType((*current_entry).address) == DetectorType::eDet) {
            event.clear();
            event.push_back(*current_entry);
            auto start_entry = current_entry--;
            auto stop_entry = current_entry++;

            while (start_entry >= std::begin(entry_buffer)) {
                if (TimeDiff(*start_entry, *current_entry) < 1500) {
                    event.push_back(*start_entry--);
                } else {
                    break;
                }
            }
            bool broke = false;
            while (stop_entry < std::end(entry_buffer) - MIN_NUM_BUFFERS) {
                if (TimeDiff(*stop_entry, *current_entry) < 1500) {
                    event.push_back(*stop_entry++);
                } else {
                    broke = true;
                    break;
                }
            }

            if (!broke) {
                first_keep = start_entry;
                break;
            }
            events.emplace_back(event);
        }
        ++current_entry;
    }
    entry_buffer.erase(std::begin(entry_buffer), first_keep);
    return events;
}

std::vector<iThembaEvent> iThembaEventBuilder::FlushEvents()
{
    std::vector<Parser::Entry_t> build_entries(std::make_move_iterator(entry_buffer.begin()),
                                               std::make_move_iterator(entry_buffer.end()));
    entry_buffer.clear();
    return BuildEvents(build_entries);
}