//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Sort.h"

#include "Histogram1D.h"
#include "Histogram2D.h"
#include "BasicStruct.h"
#include "ThreadSafeEvent.h"
#include "CommandLineInterface.h"
#include "Calibration.h"

using namespace Task;


struct usable_entries {
    size_t num;
    const word_t *words[33];

    void add(const word_t *word){ if ( num < 32 ) words[num++] = word; }
    usable_entries() : num( 0 ){}
};

struct particle_hit {
    double energy;
    double time;
    double angle;
    uint16_t ring_no;
};


struct particle_event {
    particle_hit ringHit;
    particle_hit sectHit;
    particle_hit backHit;
};

bool GetParticleEvent(const ThreadSafeEvent &buffer, particle_event &particle)
{
    auto trigger = buffer.GetTrigger();

    // If there is more than one sector within -50 and 150 ns of the trigger we will return
    for ( auto &entry : buffer.GetDetector(de_sect) ){
        auto timediff = double(entry.timestamp_raw - trigger.timestamp_raw);
        timediff += entry.cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(entry) - CalibrateTime(trigger);

        // If the time difference are within the time window, we will skip.
        if ( timediff > -50 && timediff < 150 && entry.address != trigger.address )
            return false;
    }

    // Find the prompt rings.
    usable_entries ring_entries;
    for ( auto &entry : buffer.GetDetector(de_ring) ){
        auto timediff = double(entry.timestamp_raw - trigger.timestamp_raw);
        timediff += entry.cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(entry) - CalibrateTime(trigger);

        // If the time difference are within the time window, we will skip.
        if ( timediff > -150 && timediff < 55 )
            ring_entries.add(&entry);
    }

    // We will now inspect the entries we found. If there is more than two we will not proceed.
    if ( ring_entries.num > 2 ||ring_entries.num == 0 )
        return false;

    // Next we can inspect the back detector. First we will check for prompt entries
    word_t back_entry{};
    int num_back = 0;
    for ( auto &entry : buffer.GetDetector(eDet) ){
        auto timediff = double(entry.timestamp_raw - trigger.timestamp_raw);
        timediff += entry.cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(entry) - CalibrateTime(trigger);

        // If the time difference are within the time window, we will skip.
        if ( timediff > -50 && timediff < 150 ){
            back_entry = entry;
            ++num_back;
        }
    }

    // If not only one back event, we cannot be certain about the event.
    if ( num_back != 1 )
        return false;

    particle_hit revt = {0,0,0};
    if ( ring_entries.num == 1 ){
        auto entry = ring_entries.words[0];
        auto timediff = double(entry->timestamp_raw - trigger.timestamp_raw);
        timediff += entry->cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(*entry) - CalibrateTime(trigger);
        int ringno = GetDetectorPtr(entry->address)->detectorNum;

        revt.energy = ring_entries.words[0]->energy;
        revt.time = timediff;
        revt.angle = acos(22./sqrt(22.*22. + (11.25+0.5*ringno)*(11.25+0.5*ringno)));
        revt.ring_no = ringno;
    } else if ( ring_entries.num == 2 ){
        if ( abs(GetDetectorPtr(ring_entries.words[1]->address)->detectorNum -
                 GetDetectorPtr(ring_entries.words[0]->address)->detectorNum) != 1 ){
            // The energy is the sum of the two
            revt.energy = ring_entries.words[1]->energy + ring_entries.words[0]->energy;

            // From our analysis, we have seen that the odd numbered rings above 16 is somewhat
            // strange. We will use the time of the even ring if there is two entries.
            auto as_trigger = ( (GetDetectorPtr(ring_entries.words[0]->address)->detectorNum & 1)==0 ) ? ring_entries.words[0] : ring_entries.words[1];


            auto timediff = double(as_trigger->timestamp_raw - trigger.timestamp_raw);
            timediff += as_trigger->cfdcorr - trigger.cfdcorr;
            timediff += CalibrateTime(*as_trigger) - CalibrateTime(trigger);
            double ringno = 0.5 * (GetDetectorPtr(ring_entries.words[1]->address)->detectorNum +
                                   GetDetectorPtr(ring_entries.words[0]->address)->detectorNum);

            revt.time = timediff;
            revt.angle = acos(22.0/sqrt(22.*22. + (11.25 + 0.5*ringno)*(11.25 + 0.5*ringno)));
            revt.ring_no = ( ring_entries.words[0]->energy > ring_entries.words[1]->energy ) ?
                    GetDetectorPtr(ring_entries.words[0]->address)->detectorNum :
                    GetDetectorPtr(ring_entries.words[1]->address)->detectorNum;
        }

    }

    // We now have our particle event!!
    particle.ringHit = revt;
    particle.sectHit = {trigger.energy, 0,
                        GetDetectorPtr(trigger.address)->detectorNum*0.125*M_PI,
                        GetDetectorPtr(trigger.address)->detectorNum};

    auto timediff = double(back_entry.timestamp_raw - trigger.timestamp_raw);
    timediff += back_entry.cfdcorr - trigger.cfdcorr;
    timediff += CalibrateTime(back_entry) - CalibrateTime(trigger);
    particle.backHit = {back_entry.energy, timediff,
                        GetDetectorPtr(trigger.address)->detectorNum*0.125*M_PI,
                        GetDetectorPtr(back_entry.address)->detectorNum};

    return true;
}


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
        , particle_range( histograms.Create2D("particle_range", "dE thickness", 1000, 0, 1000, "dE thickness [um]", 48, 0, 48, "Ring ID") )
        , particle_range_aligned( histograms.Create2D("particle_range_aligned", "dE thickness aligned", 1000, 0, 1000, "dE thickness [um]", 48, 0, 48, "Ring ID") )
        , front_back( histograms.Create3D("front_back", "DE vs E energy", 1000, 0, 20000, "E energy [keV]", 1000, 0, 10000, "dE energy [keV]", 48, 0, 48, "Ring ID") )
        , front_back_gated( histograms.Create3D("front_back_gated", "DE vs E energy", 1000, 0, 20000, "E energy [keV]", 1000, 0, 10000, "dE energy [keV]", 48, 0, 48, "Ring ID") )
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
    word_t trigger = buffer.GetTrigger();
    word_t *start = &trigger;
    if ( trigger.address == 0 ) {
        start = nullptr;
        /*int ref_num = 0;
        for (auto &entry: buffer.GetDetector(timeref)) {
            if (GetID(entry.address) == timeref_id) {
                ++ref_num;
                start = &entry;
            }
        }
        if (ref_num > 1)
            start = nullptr;*/
    }

    for (auto &type : {labr_3x8, labr_2x2_ss, labr_2x2_fs, de_ring, de_sect, eDet, rfchan, DetectorType::clover}){
        auto *hist = GetSpec(type);
        if ( hist )
            hist->Fill(buffer.GetDetector(type), start);
    }

    if ( buffer.GetDetector(DetectorType::de_ring).size() == 1 ){
        auto ring_word = *buffer.GetDetector(DetectorType::de_ring).begin();
        double timediff = double(ring_word.timestamp_raw - trigger.timestamp_raw);
        timediff += ring_word.cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(ring_word) - CalibrateTime(trigger);
        time_energy_ring_sect.Fill(ring_word.energy, timediff);
    }

    if ( buffer.GetDetector(DetectorType::eDet).size() == 1 ){
        auto ring_word = *buffer.GetDetector(DetectorType::de_ring).begin();
        double timediff = double(ring_word.timestamp_raw - trigger.timestamp_raw);
        timediff += ring_word.cfdcorr - trigger.cfdcorr;
        timediff += CalibrateTime(ring_word) - CalibrateTime(trigger);
        time_energy_ring_sect.Fill(ring_word.energy, timediff);
    }

    particle_event pevent{};
    if ( !GetParticleEvent(buffer, pevent) )
        return;

    // Now we will fill particle energy spectrum.
    double zrange = GetRange(pevent.sectHit.energy+pevent.backHit.energy) - GetRange(pevent.backHit.energy);
    double zrange_aligned = zrange*25.6/sqrt(25.6*25.6 + (11.25 + 0.5*pevent.ringHit.ring_no)*(11.25 + 0.5*pevent.ringHit.ring_no));
    particle_range.Fill(zrange, pevent.ringHit.ring_no);
    particle_range_aligned.Fill(zrange_aligned, pevent.ringHit.ring_no);
    front_back.Fill(pevent.backHit.energy, pevent.sectHit.energy, pevent.ringHit.ring_no);

    if ( zrange_aligned > 260 && zrange_aligned < 340 )
        front_back_gated.Fill(pevent.backHit.energy, pevent.sectHit.energy, pevent.ringHit.ring_no);

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
        } else if ( input_queue.done ){
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