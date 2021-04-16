//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_SORT_H
#define TDR2TREE_SORT_H

#include "Task.h"

#include "Histograms.h"
#include "experimentsetup.h"

struct word_t;
struct Subevent;
class Event;

namespace Task {

    struct Detector_Histograms_t
    {
        Histogram2Dp time;
        Histogram2Dp energy;
        Histogram2Dp energy_cal;
        Histogram1Dp mult;

        Detector_Histograms_t(Histograms &hist, const std::string &name, const size_t &num);

        void Fill(const word_t &word);
        void Fill(const Subevent &subvec,
                  const word_t *start = nullptr);

    };

    class HistManager {

    private:

        Histograms histograms;

        Detector_Histograms_t clover;
        Detector_Histograms_t labrL;
        Detector_Histograms_t labrS;
        Detector_Histograms_t labrF;

        Detector_Histograms_t ring;
        Detector_Histograms_t sect;
        Detector_Histograms_t back;

        //! Time energy spectra for particles.
        Histogram2Dp time_energy_sect_back;
        Histogram2Dp time_energy_ring_sect;

        Detector_Histograms_t *GetSpec(const DetectorType &type);

    public:

        HistManager();

        Histograms &GetHistograms(){ return histograms; }

        //! Fill spectra with an event
        void AddEntry(Event &buffer);

        //! Fill a single word
        void AddEntry(const word_t &word);

        //! Fill spectra directly from iterators
        template<class It>
        inline void AddEntries(It start, It stop){
            using std::placeholders::_1;
            std::for_each(start, stop, [this](const auto &p){ this->AddEntry(p); });
        }
    };

    class Sort : public Base
    {
    private:
        MCWordQueue_t &input_queue;
        HistManager hm;
        Histogram2Dp addback_hist;
    public:
        Sort(MCWordQueue_t &input, const bool &addback);

        void Run() override;

    };

}

#endif //TDR2TREE_SORT_H
