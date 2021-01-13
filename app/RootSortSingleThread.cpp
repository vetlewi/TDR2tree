//
// Created by Vetle Wegner Ingeberg on 13/01/2021.
//

#include "RootSortSingleThread.h"

#include <vector>
#include <deque>

// Buffer library
#include <Buffer/MTFileBufferFetcher.h>
#include <Buffer/STFileBufferFetcher.h>

// ROOT interface library
#include <RootInterface/RootFileManager.h>
#include <RootInterface/HistManager.h>
#include <RootInterface/TreeManager.h>

// Parser library
#include <Parser/Parser.h>

// Param library
#include <Parameters/experimentsetup.h>

#include <Utilities/CLI_interface.h>

inline double TimeDiff(const Parser::Entry_t &lhs, const Parser::Entry_t &rhs)
{
    return double(lhs.timestamp - rhs.timestamp) + (lhs.cfdcorr - rhs.cfdcorr);
}

bool SplitEntries(const Settings_t *settings, std::deque<Parser::Entry_t> &entries, std::vector<Parser::Entry_t> &event)
{
    auto entries_old = entries;
    event.clear();
    while ( !entries.empty() ){
        if ( event.empty() ){
            event.push_back(entries.front());
            entries.pop_front();
            continue;
        }

        if ( fabs(TimeDiff(event.back(), entries.front())) < settings->split_time ){
            event.push_back(entries.front());
            entries.pop_front();
            continue;
        } else {
            return true;
        }
    }
    entries = entries_old;
    return false;
}

std::vector<Event::iThembaEvent> GetEvents(const Settings_t *settings, std::vector<Parser::Entry_t> &entries)
{
    if ( settings->trigger_type == any ) {
        Event::iThembaEvent event(entries);
        return {Event::iThembaEvent(entries)};
    }

    std::vector<Event::iThembaEvent> events;
    events.reserve(16);

    auto entry = std::begin(entries);
    while ( entry != std::end(entries) ){
        if ( settings->PR271 ){
            if (entry->address == 486) {
                ++entry;
                continue;
            }
        }

        if ( GetDetectorType((*entry).address) == settings->trigger_type ) {
            auto start = entry;
            while (start != std::begin(entries)) {
                if (fabs(TimeDiff(*start, *entry)) < settings->event_time) {
                    --start;
                } else {
                    break;
                }
            }

            auto stop = entry;
            while (entry != std::end(entries)) {
                if (fabs(TimeDiff(*stop, *entry)) < settings->event_time) {
                    ++stop;
                } else {
                    break;
                }
            }
            events.emplace_back(std::vector<Parser::Entry_t>(start, stop));
        }
    }
    return events;
}

void FillROOT(const Settings_t *settings, HistManager &hm, TreeManager &tm, const std::vector<Event::iThembaEvent> &events)
{
    for ( auto event : events ){
        hm.AddEntry(event);
        if ( settings->build_tree )
            tm.AddEntry(&event);
    }
}

void ConvertROOTST(const Settings_t *settings)
{

    RootFileManager fm( settings->output_file.c_str(), "RECREATE", settings->file_title.c_str());
    HistManager hm( &fm );
    TreeManager tm( &fm,
                    settings->tree_name.c_str(),
                    settings->tree_title.c_str(),
                    settings->event_type->New());

    auto *bf = new Fetcher::MTFileBufferFetcher(settings->buffer_type);
    const Fetcher::Buffer *buf;
    std::vector<Parser::Entry_t> tmp;
    std::deque<Parser::Entry_t> entries;


    for ( auto &file : settings->input_files ){
        auto status = bf->Open(file.c_str(), 0);
        if ( !settings->running )
            break;

        while ( settings->running ){
            buf = bf->Next(status);
            if ( status != Fetcher::BufferFetcher::OKAY ){
                break;
            }
            tmp = settings->parser->GetEntry(buf);
            for ( auto &entry : tmp ){
                entries.push_back(entry);
            }

            while ( SplitEntries(settings, entries, tmp) ){
                FillROOT(settings, hm, tm, GetEvents(settings, tmp));
            }
        }


    }
    delete bf;
}