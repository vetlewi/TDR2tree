//
// Created by Vetle Wegner Ingeberg on 22/07/2022.
//

#include <TChain.h>
#include <TTree.h>

#include <iostream>
#include <string>
#include <fstream>
#include <signal.h>
#include <unistd.h>
#include "zstr.hpp"

//! Global variable signaling if the sorting has been interrupted.
static char leaveprog = 'n';

//! Signal handler for Ctrl-C.
static void keyb_int(int sig_num)
{
    leaveprog = 'y';
    if (sig_num == SIGINT) {
        printf("\n\nProgram interrupted by user...\n");
        leaveprog = 'y';
    }
}

#define OUTPUT_RAW_ENERGY 0
#define OUTPUT_CFD_FAIL 0

#if OUTPUT_RAW_ENERGY
    #if OUTPUT_CFD_FAIL
        #define CSV_SIZE ",,,,"
    #else
        #define CSV_SIZE ",,,"
    #endif // OUTPUT_CFD_FAIL
#elif OUTPUT_CFD_FAIL
    #define CSV_SIZE ",,,"
#else
    #define CSV_SIZE ",,"
#endif // OUTPUT_RAW_ENERGY

inline std::string build_name(const char *name, const char *var)
{
    return std::string(name) + std::string(var);
}

struct subevent_t {
    int id;
    double energy;
    double time;
    unsigned short energy_raw;
    bool cfd_fail;
};

template<typename T>
T &operator<<(T &stream, const subevent_t &sevt)
{
    if ( sevt.id < 0 ){
        stream << CSV_SIZE;
        return stream;
    }

    stream << sevt.id << "," << sevt.energy << "," << sevt.time;
#if OUTPUT_RAW_ENERGY
    stream << "," << sevt.energy_raw;
#endif // OUTPUT_RAW_ENERGY
#if OUTPUT_CFD_FAIL
    if ( sevt.cfd_fail )
        stream << ",1";
    else
        stream << ",0";
#endif // OUTPUT_CFD_FAIL
    return stream;
}

struct particle_event_t {
    subevent_t sectEvent;
    subevent_t backEvent;
    subevent_t ringEvent[2];

    particle_event_t()
        : sectEvent( {-1, 0, 0, 0, true} )
        , backEvent( {-1, 0, 0, 0, true} )
        , ringEvent{ {-1, 0, 0, 0, true}, {-1, 0, 0, 0, true} }{}

    static std::string csv_header() {
        return "sectID,sectE,sectT,backID,backE,backT,ringID,ringE,ringT,ringID0,ringE0,ringT0,ringID1,ringE1,ringT1";
    }
};

template<typename T>
T &operator<<(T &stream, const particle_event_t &entry)
{
    stream << entry.sectEvent << "," << entry.backEvent << ",";
    if ( entry.ringEvent[1].id < 0 )
        stream << entry.ringEvent[0].id << ",";
    else
        stream << 0.5*(entry.ringEvent[0].id + entry.ringEvent[1].id) << ",";
    stream << entry.ringEvent[0].energy + entry.ringEvent[1].energy << ",";
    stream << 0.5*(entry.ringEvent[0].time + entry.ringEvent[1].time) << ",";
    stream << entry.ringEvent[0] << "," << entry.ringEvent[1];
    return stream;
}

struct event_t {
    size_t eventID;
    particle_event_t particleEvent;
    subevent_t labrEvent;

    event_t(const size_t &_eventID,
            const particle_event_t &particle,
            const subevent_t &labr = {-1, 0, 0, 0, true} )
        : eventID( _eventID )
        , particleEvent( particle )
        , labrEvent( labr ){}

    static std::string csv_header() {
        return "eventID,"+particle_event_t::csv_header()+",labrID,labrE,labrT";
    }
};

template<typename T>
T &operator<<(T &stream, const event_t &event)
{
    stream << event.eventID << "," << event.particleEvent << "," << event.labrEvent;
    return stream;
}

struct entries_t {
    std::string _name;
    int mult;
    unsigned short id[32];
    double energy[32];
    unsigned short energy_raw[32];
    double time[32];
    bool CFDfail[32];

    entries_t(TChain *chain, const char *name)
            : _name( name )
    {
        _name[0] = std::toupper(_name[0]);
        chain->SetBranchAddress(build_name(name, "Mult").c_str(), &mult);
        chain->SetBranchAddress(build_name(name, "ID").c_str(), id);
        chain->SetBranchAddress(build_name(name, "Energy").c_str(), energy);
        chain->SetBranchAddress(build_name(name, "EnergyRaw").c_str(), energy_raw);
        chain->SetBranchAddress(build_name(name, "Time").c_str(), time);
        chain->SetBranchAddress(build_name(name, "CFDfail").c_str(), CFDfail);
    }

    subevent_t operator[](const size_t &idx) const {
        return {id[idx], energy[idx], time[idx], energy_raw[idx], CFDfail[idx]};
    }

};

template<typename T>
T &operator<<(T &stream, const entries_t &entries)
{
    stream << "\t" << entries._name << "Mult: " << entries.mult;
    for ( int i = 0 ; i < entries.mult ; ++i){
        stream << "\n\t\t" << "Hit #: " << i;
        stream << " ID: " << entries.id[i];
        stream << " Energy: " << entries.energy[i];
        stream << " Energy raw: " << entries.energy_raw[i];
        stream << " Time: " << entries.time[i];
        stream << " CFD fail: ";
        if ( entries.CFDfail[i] )
            stream << "True";
        else
            stream << "False";
    }
    return stream;
}

void analyze(TChain *chain, const char *outfile)
{
    entries_t ring_entries(chain, "ring");
    entries_t sect_entries(chain, "sect");
    entries_t back_entries(chain, "back");
    entries_t labr_entries(chain, "labrL");

    //chain->Print();

    zstr::ofstream csv_file(outfile);
    //std::stringstream csv_file;
    csv_file << event_t::csv_header() << "\n";
    int stop = 0;
    for ( int i = 0 ; i < chain->GetEntries() ; ++i ){
        if ( leaveprog == 'y' )
            break;
        auto entryNumber = i;//chain->GetEntryNumber(i);
        //if ( entryNumber < 0 )
        //	break;
        chain->GetEntry(entryNumber);
        if ( entryNumber % 10000 == 0 ){
            std::cout << "\rEntries processed: " << entryNumber << "/" << chain->GetEntries() << ", " << double(entryNumber)*100/chain->GetEntries() << "%" << ", leaveprog: " << leaveprog << std::flush;
        }

        particle_event_t pevt;

        if ( ( sect_entries.mult != 1 || back_entries.mult != 1 ) ){
            continue;
        }

        if ( ring_entries.mult == 1 ){ // Write to file now!!
            if (!( abs(sect_entries.energy[0] - ring_entries.energy[0]) < 140))
                continue;
            pevt.sectEvent = sect_entries[0];
            pevt.backEvent = back_entries[0];
            pevt.ringEvent[0] = ring_entries[0];
        } else if ( ring_entries.mult == 2 ){
            // Check if they are next to each other
            if ( abs(ring_entries.id[0] - ring_entries.id[1]) != 1 )
                continue;
            // Check that the energy matches the sector energy
            if (!( abs(sect_entries.energy[0] - (ring_entries.energy[1] + ring_entries.energy[0])) > 100 &&
                   abs(sect_entries.energy[0] - (ring_entries.energy[1] + ring_entries.energy[0])) < 375))
                continue;

            pevt.sectEvent = sect_entries[0];
            pevt.backEvent = back_entries[0];
            pevt.ringEvent[0] = ring_entries[0];
            pevt.ringEvent[1] = ring_entries[1];
        } else {
            continue;
        }

        if ( labr_entries.mult == 0 ){
            csv_file << event_t(entryNumber, pevt) << std::endl;
            continue;
        } else {
            for ( int k = 0 ; k < labr_entries.mult ; ++k ){
                csv_file << event_t(entryNumber, pevt, labr_entries[k]) << std::endl;
            }
        }

        /*std::cout << "Event #: " << i << std::endl;
        std::cout << sect_entries << std::endl;
        std::cout << back_entries << std::endl;
        std::cout << ring_entries << std::endl;
        std::cout << labr_entries << std::endl;
        if (++stop > 11)
            break;*/
    }
    std::cout << "\rEntries processed: " << chain->GetEntries() << "/" << chain->GetEntries() << ", " << double(chain->GetEntries())*100/chain->GetEntries() << "%" << std::endl;
}

int main(int argc, const char *argv[])
{
    if ( argc != 3 ){
        std::cout << "Error: Missing input and/or output" << std::endl;
        return 0;
    }

    signal(SIGINT, keyb_int); // Setting up interrupt handler (Ctrl-C)
    signal(SIGPIPE, SIG_IGN);

    TChain *chain = new TChain("events");
    chain->AddFile(argv[1]);
    analyze(chain, argv[2]);
    delete chain;
    return 0;
}