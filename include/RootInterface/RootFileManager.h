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

#ifndef ROOTFILEMANAGER_H
#define ROOTFILEMANAGER_H

#include <vector>
#include <string>

#include <TFile.h>

class TH1;
class TH2;
class TH3;
class TTree;
class TObject;

#include <vector>

#include "Histograms.h"

namespace ROOT {

    void MergeFiles(const std::vector<std::string> &infiles, const std::string &outname);

    class RootFileManager
    {

    private:

        TFile file;                         //!< File where everything will be put in.
        std::vector<TObject *> list;        //!< List to store ROOT objects.

        Histograms histograms;

        void Write();

    public:

        //! Construct and open.
        explicit RootFileManager(const char *fname, const char *mode = "RECREATE", const char *ftitle = "");

        //! Destructor.
        ~RootFileManager();

        //! Create a ROOT tree object.
        /*!
         *
         * \param name Name of the TTree object.
         * \param title Title of the tree.
         * \return A pointer to the TTree object.
         */
        TTree *CreateTree(const char *name, const char *title);

        //! Create a ROOT 1D histogram.
        /*!
         *
         * \param name Name of the TH1 object.
         * \param title Title of the histogram
         * \param xbin Number of bins on the x-axis.
         * \param xmin Lower limit of the first bin on the x-axis.
         * \param xmax Upper limit of the last bin on the x-axis.
         * \param xtitle Title of the x-axis.
         * \param dir Directory in the ROOT file to store the object.
         * \return A pointer to the TH1 object.
         */
        TH1 *CreateTH1(const char *name, const char *title, int xbin, double xmin, double xmax, const char *xtitle,
                       const char *dir = "");

        //! Create a ROOT 1D histogram from a Histogram1Dp
        TH1 *CreateTH1(Histogram1Dp h);

        //! Create a histogram
        Histogram1Dp Spec(const std::string &name, const std::string &title, const Axis::index_t &bins,
                          const Axis::bin_t &xmin, const Axis::bin_t &xmax, const std::string &xtitle)
        {
            return histograms.Create1D(name, title, bins, xmin, xmax, xtitle);
        }

        //! Creat a ROOT 2D histogram.
        /*!
         *
         * \param name Name of the TH2 object.
         * \param title Title of the histogram.
         * \param xbin Number of bins on the x-axis.
         * \param xmin Lower limit of the first bin on the x-axis.
         * \param xmax Upper limit of the last bin on the x-axis.
         * \param xtitle Title of the x-axis.
         * \param ybin Number of bins on the y-axis.
         * \param ymin Lower limit of the first bin on the y-axis.
         * \param ymax Upper limit of the last bin on the y-axis.
         * \param ytitle Title of the y-axis.
         * \param dir Directory in the ROOT file to store the object.
         * \return A pointer to the TH2 object.
         */
        TH2 *CreateTH2(const char *name, const char *title, int xbin, double xmin, double xmax, const char *xtitle,
                       int ybin, double ymin, double ymax, const char *ytitle, const char *dir = "");

        //! Create a ROOT 2D histogram from a Histogram2Dp
        TH2 *CreateTH2(Histogram2Dp h);

        //! Create a matrix
        Histogram2Dp Mat(const std::string &name, const std::string &title, const Axis::index_t &xbins,
                         const Axis::bin_t &xmin, const Axis::bin_t &xmax, const std::string &xtitle,
                         const Axis::index_t &ybins, const Axis::bin_t &ymin, const Axis::bin_t &ymax,
                         const std::string &ytitle)
        {
            return histograms.Create2D(name, title, xbins, xmin, xmax, xtitle, ybins, ymin, ymax, ytitle);
        }

        //! Creat a ROOT 3D histogram.
        /*!
         *
         * \param name Name of the TH2 object.
         * \param title Title of the histogram.
         * \param xbin Number of bins on the x-axis.
         * \param xmin Lower limit of the first bin on the x-axis.
         * \param xmax Upper limit of the last bin on the x-axis.
         * \param xtitle Title of the x-axis.
         * \param ybin Number of bins on the y-axis.
         * \param ymin Lower limit of the first bin on the y-axis.
         * \param ymax Upper limit of the last bin on the y-axis.
         * \param ytitle Title of the y-axis.
         * \param zbin Number of bins on the y-axis.
         * \param zmin Lower limit of the first bin on the y-axis.
         * \param zmax Upper limit of the last bin on the y-axis.
         * \param ztitle Title of the y-axis.
         * \param dir Directory in the ROOT file to store the object.
         * \return A pointer to the TH2 object.
         */
        TH3 *CreateTH3(const char *name, const char *title, int xbin, double xmin, double xmax, const char *xtitle,
                       int ybin, double ymin, double ymax, const char *ytitle,
                       int zbin, double zmin, double zmax, const char *ztitle, const char *dir = "");

        //! Create a ROOT 2D histogram from a Histogram2Dp
        TH3 *CreateTH3(Histogram3Dp h);

        //! Create a matrix
        Histogram3Dp Cube(const std::string &name, const std::string &title, const Axis::index_t &xbins,
                          const Axis::bin_t &xmin, const Axis::bin_t &xmax, const std::string &xtitle,
                          const Axis::index_t &ybins, const Axis::bin_t &ymin, const Axis::bin_t &ymax,
                          const std::string &ytitle, const Axis::index_t &zbins, const Axis::bin_t &zmin,
                          const Axis::bin_t &zmax, const std::string &ztitle)
        {
            return histograms.Create3D(name, title, xbins, xmin, xmax, xtitle,
                                                    ybins, ymin, ymax, ytitle,
                                                    zbins, zmin, zmax, ztitle);
        }

    };

}


#endif // ROOTFILEMANAGER_H
