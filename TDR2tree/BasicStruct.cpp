#include "BasicStruct.h"

#include <TROOT.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>

#include <algorithm>

#include <cstdint>

#include "experimentsetup.h"
#include "Calibration.h"

struct SEvent_t {
    int16_t ID;
    double energy;
    int64_t tc;
    double tf;
};

void Event::RunAddback(TH2 *ab_t_clover)
{
    // We set up a vector for each clover.
    std::vector<SEvent_t> cevent[NUM_CLOVER_DETECTORS];
    for (int16_t n = 0 ; n < clover_mult ; ++n){
        cevent[cloverID[n]/4].push_back({cloverID[n], clover_energy[n], clover_t_course[n], clover_t_fine[n]});
    }

    clover_mult = 0;
    std::vector<SEvent_t> v, v_new;
    double e, tdiff;
    for (size_t n = 0 ; n < NUM_CLOVER_DETECTORS ; ++n){
        v = cevent[n];
        std::sort(v.begin(), v.end(), [](SEvent_t lhs, SEvent_t rhs){ return lhs.energy > rhs.energy; });
        while ( v.size() > 0 ){
            v_new.clear();
            e = v[0].energy;
            for (size_t m = 1 ; m < v.size() ; ++m){
                tdiff = (v[m].tc - v[0].tc);
                tdiff += (v[m].tf - v[0].tf);
                ab_t_clover->Fill(tdiff, n);
                if ( CheckTimeGateAddback(tdiff) ){
                    e += v[m].energy;
                } else {
                    v_new.push_back(v[m]);
                }
            }
            v = v_new;
            AddClover(v[0].ID, e, v[0].tc, v[0].tf);
        }
    }

}

HistogramManager::HistogramManager()
{
    time_ring = Mat("time_ring", "Time alignment spectra, rings", 3000, -1500, 1500, "Time [ns]", NUM_SI_RING, 0, NUM_SI_RING, "Ring ID");
    time_sect = Mat("time_sect", "Time alignment spectra, sectors", 3000, -1500, 1500, "Time [ns]", NUM_SI_SECT, 0, NUM_SI_SECT, "Sector ID");
    time_back = Mat("time_back", "Time alignment spectra, E detector", 3000, -1500, 1500, "Time [ns]", NUM_SI_BACK, 0, NUM_SI_BACK, "Sector ID");
    time_labrL = Mat("time_labrL", "Time alignment spectra, large LaBr", 30000, -1500, 1500, "Time [ns]", NUM_LABR_3X8_DETECTORS, 0, NUM_LABR_3X8_DETECTORS, "Large LaBr ID");
    time_labrS = Mat("time_labrS", "Time alignment spectra, small LaBr", 30000, -1500, 1500, "Time [ns]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "Small LaBr ID");
    time_clover = Mat("time_clover", "Time alignment spectra, clover", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0, NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, "Clover ID");
    time_self_clover = Mat("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, "Time [ns]", NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS, "Clover detector");
}

HistogramManager::~HistogramManager()
{
   // Nothing to do. Histograms will be deleted when the directory
   // in which tey are stored is closed.
}

void HistogramManager::Fill(const Event *event, const Options &opt)
{
    double tdiff;
    for (int i = 0 ; i < event->labrS_mult ; ++i){
        if ( (!opt.use_all_labrS) && event->labrSID[i] != 0) // Skip unless det. 0 (our ref.)
            continue;

        for (int j = 0 ;j < event->ring_mult ; ++j){
            tdiff = event->ring_t_course[j] - event->labrS_t_course[i];
            tdiff += (event->ring_t_fine[j] - event->labrS_t_fine[i]);
            time_ring->Fill(tdiff, event->ringID[j]);
        }

        for (int j = 0 ; j < event->sect_mult ; ++j){
            tdiff = event->sect_t_course[j] - event->labrS_t_course[i];
            tdiff += (event->sect_t_fine[j] - event->labrS_t_fine[i]);
            time_sect->Fill(tdiff, event->sectID[j]);
        }

        for (int j = 0 ; j < event->back_mult ; ++j){
            tdiff = event->back_t_course[j] - event->labrS_t_course[i];
            tdiff += (event->back_t_fine[j] - event->labrS_t_fine[i]);
            time_back->Fill(tdiff, event->backID[j]);
        }

        for (int j = 0 ; j < event->labrL_mult ; ++j){
            tdiff = event->labrL_t_course[j] - event->labrS_t_course[i];
            tdiff += (event->labrL_t_fine[j] - event->labrS_t_fine[i]);
            time_labrL->Fill(tdiff, event->labrLID[j]);
        }

        for (int j = 0 ; j < event->labrS_mult ; ++j){
            tdiff = event->labrS_t_course[j] - event->labrS_t_course[i];
            tdiff += (event->labrS_t_fine[j] - event->labrS_t_fine[i]);
            time_labrS->Fill(tdiff, event->labrSID[j]);
        }

        for (int j = 0 ; j < event->clover_mult ; ++j){
            tdiff = event->clover_t_course[j] - event->labrS_t_course[i];
            tdiff += (event->clover_t_fine[j] - event->labrS_t_fine[i]);
            time_clover->Fill(tdiff, event->cloverID[j]);
        }
    }
}

TH1 *HistogramManager::Spec(const char *name, const char *title, const int &nbins, const double &xmin, const double &xmax, const char *xtitle, const char *ytitle)
{
    TH1 *h = new TH1D(name, title, nbins, xmin, xmax);
    h->GetXaxis()->SetTitle(xtitle);
    h->GetYaxis()->SetTitle(ytitle);
    return h;
}

TH2 *HistogramManager::Mat(const char *name, const char *title, const int &xbins, const double &xmin, const double &xmax, const char *xtitle, const int &ybins, const double &ymin, const double &ymax, const char *ytitle)
{
    TH2 *m = new TH2D(name, title, xbins, xmin, xmax, ybins, ymin, ymax);
    m->GetXaxis()->SetTitle(xtitle);
    m->GetYaxis()->SetTitle(ytitle);
    m->SetOption("colz");
    m->SetContour(64);
    return m;
}
