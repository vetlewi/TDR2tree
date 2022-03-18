//
// Created by Vetle Wegner Ingeberg on 19/02/2021.
//

#ifndef ROOTWRITER_H
#define ROOTWRITER_H

// -*- c++ -*-

#ifndef RootWriter_H_
#define RootWriter_H_ 1

#include <string>
#include <memory>

class TH1;
class TH2;
class TH3;
class TFile;

typedef TH1* TH1p;
typedef TH2* TH2p;
typedef TH3* TH3p;

class Named;
class Histogram1D;
class Histogram2D;
class Histogram3D;

typedef Histogram1D* Histogram1Dp;
typedef Histogram2D* Histogram2Dp;
typedef Histogram3D* Histogram3Dp;

class Histograms;

#define ROOT1D_YTITLE 1 // 0=No title on y-axis, 1=Counts/binwidth on y-axis.

//! Functions to write histograms into ROOT files.
//!
/*!
 * \class RootWriter
 * \brief Class to write hitograms in ROOT files.
 * \details This class recives a list of histograms and writes them to a root file.
 * \author unknown
 * \copyright GNU Public License v. 3
 */
class RootWriter {

private:

    //! Modify the root file to be in the correct directory before creating the histogram.
    static void Navigate( Named *named, /*!< Named object where we will find the directory. */
                          TFile *file   /*!< ROOT file we are writing to.                   */);

public:
  //! Write many histograms at once.
  /*! All of the histograms in the list will be written. The output
   *  file will be overwritten if it exists.
   */
  static void Write( Histograms& histograms,     /*!< The histogram list. */
                     const std::string& filename /*!< The output filename. */);

  //! Create a ROOT histogram from a Histogram1D.
  /*! \return the ROOT 1D histogram.
   */
  static TH1p CreateTH1(Histogram1Dp h /*!< The Histogram1D to be cpoied. */);

  //! Create a ROOT histogram from a Histogram2D.
  /*! \return the ROOT 2D histogram.
   */
  static TH2p CreateTH2(Histogram2Dp m /*!< The Histogram2D to be cpoied. */);

  //! Create a ROOT histogram from a Histogram2D.
  /*! \return the ROOT 2D histogram.
   */
  static TH3p CreateTH3(Histogram3Dp m /*!< The Histogram2D to be cpoied. */);
};

#endif /* RootWriter_H_ */

#endif  // ROOTWRITER_H
