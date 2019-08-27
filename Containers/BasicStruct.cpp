#include "BasicStruct.h"

#include <TROOT.h>
#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TTree.h>

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

/*void Event::SetupBranches(TTree *tree)
{

    tree->Branch("ring_mult", &ring_mult, "ring_mult/I");
    tree->Branch("ringID", &ringID, "ringID[ring_mult]/S");
    tree->Branch("ring_energy", &ring_energy, "ring_energy[ring_mult]/D");
    tree->Branch("ring_t_fine", &ring_t_fine, "ring_t_fine[ring_mult]/D");
    tree->Branch("ring_t_coarse",&ring_t_coarse, "ring_t_coarse[ring_mult]/L");

    // Setup sectors
    tree->Branch("sect_mult", &sect_mult, "sect_mult/I");
    tree->Branch("sectID", &sectID, "sectID[sect_mult]/S");
    tree->Branch("sect_energy", &sect_energy, "sect_energy[sect_mult]/D");
    tree->Branch("sect_t_fine", &sect_t_fine, "sect_t_fine[sect_mult]/D");
    tree->Branch("sect_t_coarse", &sect_t_coarse, "sect_t_coarse[sect_mult]/L");

    // Setup back
    tree->Branch("back_mult", &back_mult, "back_mult/I");
    tree->Branch("backID", &backID, "backID[back_mult]/S");
    tree->Branch("back_energy", &back_energy, "back_energy[back_mult]/D");
    tree->Branch("back_t_fine", &back_t_fine, "back_t_fine[back_mult]/D");
    tree->Branch("back_t_coarse", &back_t_coarse, "back_t_coarse[back_mult]/L");

    // Setup labr L
    tree->Branch("labrL_mult", &labrL_mult, "labrL_mult/I");
    tree->Branch("labrLID", &labrLID, "labrLID[labrL_mult]/S");
    tree->Branch("labrL_energy", &labrL_energy, "labrL_energy[labrL_mult]/D");
    tree->Branch("labrL_t_fine", &labrL_t_fine, "labrL_t_fine[labrL_mult]/D");
    tree->Branch("labrL_t_coarse", &labrL_t_coarse, "labrL_t_coarse[labrL_mult]/L");

    // Setup labr S
    tree->Branch("labrS_mult", &labrS_mult, "labrS_mult/I");
    tree->Branch("labrSID", &labrSID, "labrSID[labrS_mult]/S");
    tree->Branch("labrS_energy", &labrS_energy, "labrS_energy[labrS_mult]/D");
    tree->Branch("labrS_t_fine", &labrS_t_fine, "labrS_t_fine[labrS_mult]/D");
    tree->Branch("labrS_t_coarse", &labrS_t_coarse, "labrS_t_coarse[labrS_mult]/L");

    // Setup labr F
    tree->Branch("labrF_mult", &labrF_mult, "labrF_mult/I");
    tree->Branch("labrFID", &labrFID, "labrFID[labrF_mult]/S");
    tree->Branch("labrF_energy", &labrF_energy, "labrF_energy[labrF_mult]/D");
    tree->Branch("labrF_t_fine", &labrF_t_fine, "labrF_t_fine[labrF_mult]/D");
    tree->Branch("labrF_t_coarse", &labrF_t_coarse, "labrF_t_coarse[labrF_mult]/L");

    // Setup RF
    tree->Branch("rfMult", &rfMult, "rfMult/I");
    tree->Branch("RF_t_coarse", &RF_t_coarse, "RF_t_coarse[rfMult]/L");
    tree->Branch("RF_t_fine", &RF_t_fine, "RF_t_fine[rfMult]/D");

    // Setup clover
    tree->Branch("clover_mult",&clover_mult, "clover_mult/I");
    tree->Branch("cloverID",&cloverID, "cloverID[clover_mult]/S");
    tree->Branch("clover_energy", &clover_energy, "clover_energy[clover_mult]/D");
    tree->Branch("clover_t_fine", &clover_t_fine, "clover_t_fine[clover_mult]/D");
    tree->Branch("clover_t_coarse", &clover_t_coarse, "clover_t_coarse[clover_mult]/L");
    tree->BranchRef();

}*/

/*HistogramManager::HistogramManager()
{
    time_ring = Mat("time_ring", "Time alignment spectra, rings", 3000, -1500, 1500, "Time [ns]", NUM_SI_RING, 0, NUM_SI_RING, "Ring ID");
    time_sect = Mat("time_sect", "Time alignment spectra, sectors", 3000, -1500, 1500, "Time [ns]", NUM_SI_SECT, 0, NUM_SI_SECT, "Sector ID");
    time_back = Mat("time_back", "Time alignment spectra, E detector", 3000, -1500, 1500, "Time [ns]", NUM_SI_BACK, 0, NUM_SI_BACK, "Sector ID");
    time_labrL = Mat("time_labrL", "Time alignment spectra, large LaBr", 30000, -1500, 1500, "Time [ns]", NUM_LABR_3X8_DETECTORS, 0, NUM_LABR_3X8_DETECTORS, "Large LaBr ID");
    time_labrS = Mat("time_labrS", "Time alignment spectra, small LaBr (slow signal)", 30000, -1500, 1500, "Time [ns]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "Small LaBr ID");
    time_labrF = Mat("time_labrF", "Time alignment spectra, small LaBr (fast signal)", 30000, -1500, 1500, "Time [ns]", NUM_LABR_2X2_DETECTORS, 0, NUM_LABR_2X2_DETECTORS, "Small LaBr ID");
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
    for (int i = 0 ; i < event->rfMult ; ++i){

        for (int j = 0 ;j < event->ring_mult ; ++j){
            tdiff = event->ring_t_coarse[j] - event->RF_t_coarse[i];
            tdiff += (event->ring_t_fine[j] - event->RF_t_fine[i]);
            time_ring->Fill(tdiff, event->ringID[j]);
        }

        for (int j = 0 ; j < event->sect_mult ; ++j){
            tdiff = event->sect_t_coarse[j] - event->RF_t_coarse[i];
            tdiff += (event->sect_t_fine[j] - event->RF_t_fine[i]);
            time_sect->Fill(tdiff, event->sectID[j]);
        }

        for (int j = 0 ; j < event->back_mult ; ++j){
            tdiff = event->back_t_coarse[j] - event->RF_t_coarse[i];
            tdiff += (event->back_t_fine[j] - event->RF_t_fine[i]);
            time_back->Fill(tdiff, event->backID[j]);
        }

        for (int j = 0 ; j < event->labrL_mult ; ++j){
            tdiff = event->labrL_t_coarse[j] - event->RF_t_coarse[i];
            tdiff += (event->labrL_t_fine[j] - event->RF_t_fine[i]);
            time_labrL->Fill(tdiff, event->labrLID[j]);
        }

        for (int j = 0 ; j < event->labrS_mult ; ++j){
            tdiff = event->labrS_t_coarse[j] - event->RF_t_coarse[i];
            tdiff += (event->labrS_t_fine[j] - event->RF_t_fine[i]);
            time_labrS->Fill(tdiff, event->labrSID[j]);
        }

        for (int j = 0 ; j < event->labrF_mult ; ++j){
            tdiff = event->labrF_t_coarse[j] - event->RF_t_coarse[i];
            tdiff += (event->labrF_t_fine[j] - event->RF_t_fine[i]);
            time_labrF->Fill(tdiff, event->labrFID[j]);
        }

        for (int j = 0 ; j < event->clover_mult ; ++j){
            tdiff = event->clover_t_coarse[j] - event->RF_t_coarse[i];
            tdiff += (event->clover_t_fine[j] - event->RF_t_fine[i]);
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
*/