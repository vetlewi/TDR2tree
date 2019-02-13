//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Feb 12 10:08:10 2019 by ROOT version 6.16/00
// from TTree events/events
// found on file: /Users/vetlewi/test.root
//////////////////////////////////////////////////////////

#ifndef selector_h
#define selector_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TTreeReaderArray.h>
#include <TH1.h>
#include <TH2.h>

// Headers needed by this particular selector


class selector : public TSelector {
public :
   TTreeReader     fReader;  //!the tree reader
   TTree          *fChain = 0;   //!pointer to the analyzed TTree or TChain

   TH2 *ede[48];

   // Readers to access the data (delete the ones you do not need).
   TTreeReaderValue<Short_t> ringID = {fReader, "ringID"};
   TTreeReaderValue<Double_t> ring_energy = {fReader, "ring_energy"};
   TTreeReaderValue<Double_t> ring_t_fine = {fReader, "ring_t_fine"};
   TTreeReaderValue<Long64_t> ring_t_course = {fReader, "ring_t_course"};
   TTreeReaderValue<Short_t> sect_mult = {fReader, "sect_mult"};
   TTreeReaderArray<Short_t> sectID = {fReader, "sectID"};
   TTreeReaderArray<Double_t> sect_energy = {fReader, "sect_energy"};
   TTreeReaderArray<Double_t> sect_t_fine = {fReader, "sect_t_fine"};
   TTreeReaderArray<Long64_t> sect_t_course = {fReader, "sect_t_course"};
   TTreeReaderValue<Short_t> back_mult = {fReader, "back_mult"};
   TTreeReaderArray<Short_t> backID = {fReader, "backID"};
   TTreeReaderArray<Double_t> back_energy = {fReader, "back_energy"};
   TTreeReaderArray<Double_t> back_t_fine = {fReader, "back_t_fine"};
   TTreeReaderArray<Long64_t> back_t_course = {fReader, "back_t_course"};
   TTreeReaderValue<Short_t> labrL_mult = {fReader, "labrL_mult"};
   TTreeReaderArray<Short_t> labrLID = {fReader, "labrLID"};
   TTreeReaderArray<Double_t> labrL_energy = {fReader, "labrL_energy"};
   TTreeReaderArray<Double_t> labrL_t_fine = {fReader, "labrL_t_fine"};
   TTreeReaderArray<Long64_t> labrL_t_course = {fReader, "labrL_t_course"};
   TTreeReaderValue<Short_t> labrS_mult = {fReader, "labrS_mult"};
   TTreeReaderArray<Short_t> labrSID = {fReader, "labrSID"};
   TTreeReaderArray<Double_t> labrS_energy = {fReader, "labrS_energy"};
   TTreeReaderArray<Double_t> labrS_t_fine = {fReader, "labrS_t_fine"};
   TTreeReaderArray<Long64_t> labrS_t_course = {fReader, "labrS_t_course"};
   TTreeReaderValue<Short_t> clover_mult = {fReader, "clover_mult"};
   TTreeReaderArray<Short_t> cloverID = {fReader, "cloverID"};
   TTreeReaderArray<Short_t> clover_crystal = {fReader, "clover_crystal"};
   TTreeReaderArray<Double_t> clover_energy = {fReader, "clover_energy"};
   TTreeReaderArray<Double_t> clover_t_fine = {fReader, "clover_t_fine"};
   TTreeReaderArray<Long64_t> clover_t_course = {fReader, "clover_t_course"};


   selector(TTree * /*tree*/ =0) { }
   virtual ~selector() { }
   virtual Int_t   Version() const { return 2; }
   virtual void    Begin(TTree *tree);
   virtual void    SlaveBegin(TTree *tree);
   virtual void    Init(TTree *tree);
   virtual Bool_t  Notify();
   virtual Bool_t  Process(Long64_t entry);
   virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
   virtual void    SetOption(const char *option) { fOption = option; }
   virtual void    SetObject(TObject *obj) { fObject = obj; }
   virtual void    SetInputList(TList *input) { fInput = input; }
   virtual TList  *GetOutputList() const { return fOutput; }
   virtual void    SlaveTerminate();
   virtual void    Terminate();

   ClassDef(selector,0);

};

#endif

#ifdef selector_cxx
void selector::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the reader is initialized.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   fReader.SetTree(tree);
}

Bool_t selector::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}


#endif // #ifdef selector_cxx
