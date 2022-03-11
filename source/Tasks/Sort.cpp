//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Sort.h"

#include "Histogram1D.h"
#include "Histogram2D.h"
#include "BasicStruct.h"
#include "ThreadSafeEvent.h"
#include "CommandLineInterface.h"

using namespace Task;

Detector_Histograms_t::Detector_Histograms_t(ThreadSafeHistograms &hm, const std::string &name, const size_t &num)
        : time( hm.Create2D(std::string("time_"+name), std::string("Time spectra "+name), 3000, -1500, 1500, "Time [ns]", num, 0, num, std::string(name+" ID").c_str()) )
        , energy( hm.Create2D(std::string("energy_"+name), std::string("Energy spectra "+name), 65536, 0, 65536, "Energy [ch]", num, 0, num, std::string(name+" ID").c_str()) )
        , energy_cal( hm.Create2D(std::string("energy_cal_"+name), std::string("energy spectra "+name+" (cal)"), 16384, 0, 16384, "Energy [keV]", num, 0, num, std::string(name+" ID").c_str()) )
        , mult( hm.Create1D(std::string("mult_"+name), std::string("Multiplicity " + name), 128, 0, 128, "Multiplicity") )
{}

void Detector_Histograms_t::Fill(const word_t &word)
{
    auto dno = GetID(word.address);
    energy.Fill(word.adcdata, dno);
    energy_cal.Fill(word.energy, dno);
}

void Detector_Histograms_t::Fill(const Subevent &subvec, const word_t *start)
{
    int dno;
    mult.Fill(subvec.size());
    for ( auto &entry : subvec ){
        dno = GetID(entry.address);
        energy.Fill(entry.adcdata, dno);
        energy_cal.Fill(entry.energy, dno);
        if ( start )
            time.Fill(double(entry.timestamp - start->timestamp) + (entry.cfdcorr - start->cfdcorr), dno);
    }
}

Task::HistManager::HistManager(ThreadSafeHistograms &histograms)
        : clover( histograms, "clover", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS )
        , labrL( histograms, "labrL", NUM_LABR_3X8_DETECTORS )
        , labrS( histograms, "labrS", NUM_LABR_2X2_DETECTORS )
        , labrF( histograms, "labrF", NUM_LABR_2X2_DETECTORS )
        , ring( histograms, "ring", NUM_SI_RING )
        , sect( histograms, "sect", NUM_SI_SECT )
        , back( histograms, "back", NUM_SI_BACK )
        , time_energy_sect_back( histograms.Create2D("time_energy_sect_back", "Energy vs. sector/back time", 1000, 0, 30000, "E energy [keV]", 3000, -1500, 1500, "t_{back} - t_{sector} [ns]") )
        , time_energy_ring_sect( histograms.Create2D("time_energy_ring_sect", "Energy vs. sector/back time", 1000, 0, 30000, "Sector energy [keV]", 3000, -1500, 1500, "t_{ring} - t_{sector} [ns]") )
{
}

Detector_Histograms_t *Task::HistManager::GetSpec(const DetectorType &type)
{
    switch (type) {
        case DetectorType::labr_3x8 : return &labrL;
        case DetectorType::labr_2x2_ss : return &labrS;
        case DetectorType::labr_2x2_fs : return &labrF;
        case DetectorType::clover : return &clover;
        case DetectorType::de_ring : return &ring;
        case DetectorType::de_sect : return &sect;
        case DetectorType::eDet : return &back;
        default: return nullptr;
    }
}

void Task::HistManager::AddEntry(ThreadSafeEvent &buffer)
{

    // LaBr 0 is our time reference. We only use it if there is only one of that type.
    const word_t *start = nullptr;
    int ref_num = 0;
    for ( auto &entry : buffer.GetDetector(DetectorType::labr_2x2_fs) ){
        if ( GetID(entry.address) == 0 ){
            ++ref_num;
            start = &entry;
        }
    }

    if (ref_num != 1)
        start = nullptr;

    for (auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, DetectorType::clover}){
        auto *hist = GetSpec(type);
        if ( hist )
            hist->Fill(buffer.GetDetector(type), start);
    }

    for ( auto sect_evt : buffer.GetDetector(DetectorType::de_sect) ){

        for ( auto ring_evt : buffer.GetDetector(DetectorType::de_ring) ){
            time_energy_ring_sect.Fill(
                    ring_evt.energy,
                    double(ring_evt.timestamp - sect_evt.timestamp) + (ring_evt.cfdcorr - sect_evt.cfdcorr));
        }

        for ( auto back_evt : buffer.GetDetector(DetectorType::eDet) ){
            time_energy_sect_back.Fill(
                    back_evt.energy,
                    double(back_evt.timestamp - sect_evt.timestamp) + (back_evt.cfdcorr - sect_evt.cfdcorr));
        }

    }
}

void Task::HistManager::AddEntry(const word_t &word)
{
    auto *spec = GetSpec(GetDetector(word.address).type);
    if ( spec )
        spec->Fill(word);
}

Sort::Sort(TEWordQueue_t &input, ThreadSafeHistograms &histograms, const bool &addback)
    : input_queue( input )
    , hm( histograms )
    , do_addback( addback )
    , addback_hist( histograms.Create2D("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector") )
{}

void Sort::Run()
{
    Triggered_event input;
    while ( true ){
        if ( input_queue.wait_dequeue_timed(input, std::chrono::microseconds(100)) ){
            ThreadSafeEvent event(input.trigger, input.entries, ( do_addback ) ? &addback_hist : nullptr);
            hm.AddEntry(event);
        } else if ( done ){
            break;
        } else {
            std::this_thread::yield();
        }
    }
}

Sorters::Sorters(TEWordQueue_t &input, const CLI::Options &options, const size_t &no_workers)
    : input_queue( input )
    , histograms( )
    , sorters( no_workers )
{
    for ( int n = 0 ; n < no_workers ; ++n ){
        sorters[n] = new Sort(input, histograms, options.addback.value());
    }
}

Sorters::~Sorters()
{
    for ( auto s : sorters )
        delete s;
}