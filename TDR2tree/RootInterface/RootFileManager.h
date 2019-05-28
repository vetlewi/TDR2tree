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

#include <TFile.h>

class TH1;
class TH2;
class TTree;
class TObject;

#include <vector>

class RootFileManager
{

private:

    TFile file;                         //!< File where everything will be put in.
    std::vector<TObject *> list;        //!< List to store ROOT objects.


public:

    //! Construct and open.
    explicit RootFileManager(const char *fname, const char *mode="RECREATE", const char *ftitle="");

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
    TH1 *CreateTH1(const char *name, const char *title, int xbin, double xmin, double xmax, const char *xtitle, const char *dir="");

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
                   int ybin, double ymin, double ymax, const char *ytitle, const char *dir="");

};


#endif // ROOTFILEMANAGER_H
