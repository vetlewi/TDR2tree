/*!
 * \file RootWriter.cpp
 * \brief Implementation of RootWriter.
 * \author unknown
 * \copyright GNU Public License v. 3
 */

#include "RootWriter.h"

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

#include "Histogram1D.h"
#include "Histogram2D.h"

// ########################################################################

void RootWriter::Write( Histograms& histograms,
                        const std::string& filename )
{
  TFile outfile( filename.c_str(), "recreate" );

  Histograms::list1d_t list1d = histograms.GetAll1D();
  for(auto & it : list1d)
    CreateTH1( it );

  Histograms::list2d_t list2d = histograms.GetAll2D();
  for(auto & it : list2d)
    CreateTH2( it );

  outfile.Write();
  outfile.Close();
}

// ########################################################################

TH1p RootWriter::CreateTH1(Histogram1Dp h)
{
  const Axis& xax = h->GetAxisX();
  const int channels = xax.GetBinCount();
  TH1* r = new TH1I( h->GetName().c_str(), h->GetTitle().c_str(),
                     channels, xax.GetLeft(), xax.GetRight() );

  TAxis* rxax = r->GetXaxis();
  rxax->SetTitle(xax.GetTitle().c_str());
  rxax->SetTitleSize(0.03);
  rxax->SetLabelSize(0.03);

  TAxis* ryax = r->GetYaxis();
#if ROOT1D_YTITLE
  char buf[8];
    std::sprintf(buf, "%.2f", xax.GetBinWidth());
    std::string ytitle = "Counts/" + std::string(buf);
    ryax->SetTitle(ytitle.c_str());
    ryax->SetTitleSize(0.03);
#endif // ROOT1D_YTITLE
  ryax->SetLabelSize(0.03);

  for(int i=0; i<channels+2; ++i)
    r->SetBinContent(i, h->GetBinContent(i));
  r->SetEntries( h->GetEntries() );

  return r;
}

// ########################################################################

TH2* RootWriter::CreateTH2(Histogram2Dp h)
{
  const Axis& xax = h->GetAxisX();
  const Axis& yax = h->GetAxisY();
  const int xchannels = xax.GetBinCount();
  const int ychannels = yax.GetBinCount();
  TH2* mat = new TH2F( h->GetName().c_str(), h->GetTitle().c_str(),
                       xchannels, xax.GetLeft(), xax.GetRight(),
                       ychannels, yax.GetLeft(), yax.GetRight() );
  mat->SetOption( "colz" );
  mat->SetContour( 64 );

  TAxis* rxax = mat->GetXaxis();
  rxax->SetTitle(xax.GetTitle().c_str());
  rxax->SetTitleSize(0.03);
  rxax->SetLabelSize(0.03);

  TAxis* ryax = mat->GetYaxis();
  ryax->SetTitle(yax.GetTitle().c_str());
  ryax->SetTitleSize(0.03);
  ryax->SetLabelSize(0.03);
  ryax->SetTitleOffset(1.3);

  TAxis* zax = mat->GetZaxis();
  zax->SetLabelSize(0.025);

  for(int iy=0; iy<ychannels+2; ++iy)
    for(int ix=0; ix<xchannels+2; ++ix)
      mat->SetBinContent(ix, iy, h->GetBinContent(ix, iy));
  mat->SetEntries( h->GetEntries() );

  return mat;
}