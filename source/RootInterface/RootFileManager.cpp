/*******************************************************************************
 * Copyright (C) 2019 Vetle W. Ingeberg                                        *
 * Author: Vetle Wegner Ingeberg, vetlewi@fys.uio.no                           *
 *                                                                             *
 * --------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the       *
 * Free Software Foundation; either version 3 of the license, or (at your      *
 * option) any later version.                                                  *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but         *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General   *
 * Public License for more details.                                            *
 *                                                                             *
 * You should have recived a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                    *
 *                                                                             *
 *******************************************************************************/

#include "RootFileManager.h"

#include <TFileMerger.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>

#include "Histogram1D.h"
#include "Histogram2D.h"
#include "Histogram3D.h"


void ROOT::MergeFiles(const std::vector<std::string> &infiles, const std::string &outname)
{
    TFileMerger merger;
    merger.OutputFile(outname.c_str());
    for ( auto &file : infiles )
        merger.AddFile(file.c_str(), false);
    merger.Merge();
}

ROOT::RootFileManager::RootFileManager(const char *fname, const char *mode, const char *ftitle)
    : file( fname, mode, ftitle )
{
    //list.SetOwner(false);
}

ROOT::RootFileManager::~RootFileManager()
{
    Write();
    file.Write();
    file.Close();
}

void ROOT::RootFileManager::Write()
{
    for ( auto &hist : histograms.GetAll1D() ){
        CreateTH1(hist);
    }

    for ( auto &mat : histograms.GetAll2D() ){
        CreateTH2(mat);
    }

    for ( auto &cube : histograms.GetAll3D() ){
        CreateTH3(cube);
    }
}

TTree *ROOT::RootFileManager::CreateTree(const char *name, const char *title)
{
    auto *tree = new TTree(name, title);
    list.push_back(tree);
    return tree;
}

TH1 *ROOT::RootFileManager::CreateTH1(const char *name, const char *title, int xbin, double xmin, double xmax, const char *xtitle, const char *dir)
{
    if ( !file.GetDirectory(dir) ){
        file.mkdir(dir, dir);
    }
    file.cd(dir);
    TH1 *h = new TH1I(name, title, xbin, xmin, xmax);
    h->GetXaxis()->SetTitle(xtitle);
    h->GetXaxis()->SetTitleSize(0.03);
    h->GetXaxis()->SetLabelSize(0.03);
    file.cd();
    list.push_back(h);
    return h;
}

TH1 *ROOT::RootFileManager::CreateTH1(Histogram1Dp h)
{
    const Axis& xax = h->GetAxisX();
    const int channels = xax.GetBinCount();
    file.cd("");
    TH1* r = new TH1I( h->GetName().c_str(), h->GetTitle().c_str(),
                       channels, xax.GetLeft(), xax.GetRight() );
    TAxis* rxax = r->GetXaxis();
    rxax->SetTitle(xax.GetTitle().c_str());
    rxax->SetTitleSize(0.03);
    rxax->SetLabelSize(0.03);

    TAxis* ryax = r->GetYaxis();
    ryax->SetLabelSize(0.03);

    for(int i=0; i<channels+2; ++i)
        r->SetBinContent(i, h->GetBinContent(i));
    r->SetEntries( h->GetEntries() );
    file.cd();
    list.push_back(r);
    return r;
}

TH2 *
ROOT::RootFileManager::CreateTH2(const char *name, const char *title, int xbin, double xmin, double xmax, const char *xtitle, int ybin, double ymin, double ymax, const char *ytitle, const char *dir)
{
    if ( !file.GetDirectory(dir) ){
        file.mkdir(dir, dir);
    }
    file.cd(dir);
    TH2 *m = new TH2I(name, title, xbin, xmin, xmax, ybin, ymin, ymax);
    m->GetXaxis()->SetTitle(xtitle);
    m->GetXaxis()->SetTitleSize(0.03);
    m->GetXaxis()->SetLabelSize(0.03);
    m->GetYaxis()->SetTitle(ytitle);
    m->GetYaxis()->SetTitleSize(0.03);
    m->GetYaxis()->SetLabelSize(0.03);
    m->GetYaxis()->SetTitleOffset(1.3);
    m->GetZaxis()->SetLabelSize(0.025);
    m->SetOption("colz");
    m->SetContour(64);
    file.cd();
    list.push_back(m);
    return m;
}

TH2 *ROOT::RootFileManager::CreateTH2(Histogram2Dp h)
{
    const Axis& xax = h->GetAxisX();
    const Axis& yax = h->GetAxisY();
    const int xchannels = xax.GetBinCount();
    const int ychannels = yax.GetBinCount();
    file.cd("");
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
    file.cd();
    list.push_back(mat);
    return mat;
}

TH3 *
ROOT::RootFileManager::CreateTH3(const char *name, const char *title,
                                 int xbin, double xmin, double xmax, const char *xtitle,
                                 int ybin, double ymin, double ymax, const char *ytitle,
                                 int zbin, double zmin, double zmax, const char *ztitle, const char *dir)
{
    if ( !file.GetDirectory(dir) ){
        file.mkdir(dir, dir);
    }
    file.cd(dir);
    TH3 *m = new TH3I(name, title, xbin, xmin, xmax,
                                   ybin, ymin, ymax,
                                   zbin, zmin, zmax);
    m->GetXaxis()->SetTitle(xtitle);
    m->GetXaxis()->SetTitleSize(0.03);
    m->GetXaxis()->SetLabelSize(0.03);
    m->GetYaxis()->SetTitle(ytitle);
    m->GetYaxis()->SetTitleSize(0.03);
    m->GetYaxis()->SetLabelSize(0.03);
    m->GetYaxis()->SetTitleOffset(1.3);
    m->GetZaxis()->SetTitle(ztitle);
    m->GetZaxis()->SetLabelSize(0.025);
    m->SetOption("colz");
    m->SetContour(64);
    file.cd();
    list.push_back(m);
    return m;
}

TH3 *ROOT::RootFileManager::CreateTH3(Histogram3Dp h)
{
    const Axis& xax = h->GetAxisX();
    const Axis& yax = h->GetAxisY();
    const Axis& zax = h->GetAxisZ();
    const auto xchannels = xax.GetBinCount();
    const auto ychannels = yax.GetBinCount();
    const auto zchannels = zax.GetBinCount();
    file.cd("");
    TH3* cube = new TH3F( h->GetName().c_str(), h->GetTitle().c_str(),
                         xchannels, xax.GetLeft(), xax.GetRight(),
                         ychannels, yax.GetLeft(), yax.GetRight(),
                         zchannels, zax.GetLeft(), zax.GetRight());
    cube->SetOption( "colz" );
    cube->SetContour( 64 );

    TAxis* rxax = cube->GetXaxis();
    rxax->SetTitle(xax.GetTitle().c_str());
    rxax->SetTitleSize(0.03);
    rxax->SetLabelSize(0.03);

    TAxis* ryax = cube->GetYaxis();
    ryax->SetTitle(yax.GetTitle().c_str());
    ryax->SetTitleSize(0.03);
    ryax->SetLabelSize(0.03);
    ryax->SetTitleOffset(1.3);

    TAxis* rzax = cube->GetZaxis();
    ryax->SetLabelSize(0.025);
    rzax->SetTitle(zax.GetTitle().c_str());

    for(int iz=0; iz<zchannels+2; ++iz)
        for(int iy=0; iy<ychannels+2; ++iy)
            for(int ix=0; ix<xchannels+2; ++ix)
                cube->SetBinContent(ix, iy, iz, h->GetBinContent(ix, iy, iz));
    cube->SetEntries( h->GetEntries() );
    file.cd();
    list.push_back(cube);
    return cube;
}




