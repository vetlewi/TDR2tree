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
    time_ring = new TH2D("time_ring", "Time alignment spectra, rings", 3000, -1500, 1500, NUM_SI_RING, 0, NUM_SI_RING);
    time_ring->GetXaxis()->SetTitle("Time ring [ns]");
    time_ring->GetYaxis()->SetTitle("Ring ID");
    time_ring->GetZaxis()->SetTitle("Counts/1 ns");
    time_ring->SetOption( "colz" );

    time_sect = new TH2D("time_sect", "Time alignment spectra, sectors", 3000, -1500, 1000, NUM_SI_SECT, 0, NUM_SI_SECT);
    time_sect->GetXaxis()->SetTitle("Time sector [ns]");
    time_sect->GetYaxis()->SetTitle("Sector ID");
    time_sect->GetZaxis()->SetTitle("Counts/1 ns");
    time_sect->SetOption( "colz" );

    time_back = new TH2D("time_back", "Time alignment spectra, back", 3000, -1500, 1500, NUM_SI_BACK, 0, NUM_SI_BACK);
    time_back->GetXaxis()->SetTitle("Time back [ns]");
    time_back->GetYaxis()->SetTitle("Back ID");
    time_back->GetZaxis()->SetTitle("Counts/1 ns");
    time_back->SetOption( "colz" );

    time_labrL = new TH2D("time_labrL", "Time alignment spectra, large LaBr", 30000, -1500, 1500, NUM_LABR_3X8_DETECTORS, 0, NUM_LABR_3X8_DETECTORS);
    time_labrL->GetXaxis()->SetTitle("Time LaBr [ns]");
    time_labrL->GetYaxis()->SetTitle("LaBr ID");
    time_labrL->GetZaxis()->SetTitle("Counts/0.1 ns");
    time_labrL->SetOption( "colz" );

    time_labrS = new TH2D("time_labrS", "Time alignment spectra, small LaBr", 30000, -1500, 1500, NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS);
    time_labrS->GetXaxis()->SetTitle("Time LaBr [ns]");
    time_labrS->GetYaxis()->SetTitle("LaBr ID");
    time_labrS->GetZaxis()->SetTitle("Counts/0.1 ns");
    time_labrS->SetOption( "colz" );

    time_clover = new TH2D("time_clover", "Time alignment spectra, clover", 3000, -1500, 1500, NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS, 0, NUM_CLOVER_DETECTORS*NUM_CLOVER_CRYSTALS);
    time_clover->GetXaxis()->SetTitle("Time clover [ns]");
    time_clover->GetYaxis()->SetTitle("Clover ID");
    time_clover->GetZaxis()->SetTitle("Counts/1 ns");
    time_clover->SetOption( "colz" );

    time_self_clover = new TH2D("time_self_clover", "Time spectra, clover self timing", 3000, -1500, 1500, NUM_CLOVER_DETECTORS, 0, NUM_CLOVER_DETECTORS);
    time_self_clover->GetXaxis()->SetTitle("Time clover [ns]");
    time_self_clover->GetYaxis()->SetTitle("Clover ID");
    time_self_clover->GetZaxis()->SetTitle("Counts/1 ns");
    time_self_clover->SetOption( "colz" );
}

HistogramManager::~HistogramManager()
{
   // Nothing to do. Histograms will be deleted when the directory
   // in which tey are stored is closed.
}

void HistogramManager::Fill(const Event *event)
{
    double tdiff;
    for (int i = 0 ; i < event->labrS_mult ; ++i){
        if (event->labrSID[i] != 0) // Skip unless det. 0 (our ref.)
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
