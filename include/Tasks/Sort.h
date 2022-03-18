//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_SORT_H
#define TDR2TREE_SORT_H

#include <Task.h>

#include <Histograms.h>
#include <ThreadSafeHistograms.h>
#include <experimentsetup.h>
#include <CommandLineInterface.h>

struct word_t;
struct Subevent;
class ThreadSafeEvent;

namespace Task {

    struct Detector_Histograms_t
    {
        ThreadSafeHistogram2D time;
        ThreadSafeHistogram2D energy;
        ThreadSafeHistogram2D energy_cal;
        ThreadSafeHistogram1D mult;

        Detector_Histograms_t(ThreadSafeHistograms &hist, const std::string &name, const size_t &num);

        void Fill(const word_t &word);
        void Fill(const Subevent &subvec,
                  const word_t *start = nullptr);

    };

    class HistManager {

    private:

        Detector_Histograms_t clover;
        Detector_Histograms_t labrL;
        Detector_Histograms_t labrS;
        Detector_Histograms_t labrF;

        Detector_Histograms_t ring;
        Detector_Histograms_t sect;
        Detector_Histograms_t back;

        //! Time energy spectra for particles.
        ThreadSafeHistogram2D time_energy_sect_back;
        ThreadSafeHistogram2D time_energy_ring_sect;

        ThreadSafeHistogram2D particle_range, particle_range_aligned;
        ThreadSafeHistogram3D front_back;
        ThreadSafeHistogram3D front_back_gated;

        Detector_Histograms_t *GetSpec(const DetectorType &type);

        void FillSectorTrigger(ThreadSafeEvent &buffer);

    public:

        HistManager(ThreadSafeHistograms &histograms);

        //Histograms &GetHistograms(){ return histograms; }

        //! Fill spectra with an event
        void AddEntry(ThreadSafeEvent &buffer);

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
        TEWordQueue_t &input_queue;
        HistManager hm;
        const bool do_addback;
        ThreadSafeHistogram2D addback_hist;

    public:
        Sort(TEWordQueue_t &input, ThreadSafeHistograms &histograms, const bool &addback);
        //Histograms &GetHistograms(){ return hm.GetHistograms(); }
        void Run() override;

    };

    class Sorters {
    private:
        TEWordQueue_t &input_queue;
        ThreadSafeHistograms histograms;
        std::vector<Sort *> sorters;

    public:
        Sorters(TEWordQueue_t &input, const CLI::Options &options, const size_t &no_workers = 4);
        ~Sorters();

        std::vector<Sort *>::iterator begin(){ return sorters.begin(); }
        std::vector<Sort *>::iterator end(){ return sorters.end(); }

        Histograms &GetHistogram(){ return histograms.GetHistograms(); }

    };

    /*!
     * Gather and merge histograms.
     * \param sorters
     * \param outfile
     */
    extern void Gather(std::vector<Sort> &sorters, const char *outfile);

}

#endif //TDR2TREE_SORT_H
