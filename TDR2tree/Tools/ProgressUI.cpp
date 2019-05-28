//
// Created by Vetle Wegner Ingeberg on 2019-05-28.
//

#include "ProgressUI.h"

#include <iostream>

void ProgressUI::StartNewFile(const std::string &fname, const size_t &size)
{
    curr_fname = fname;
    length = size;
    shown = 0;
    std::cout << "                                                                                            " << "\r";
    std::cout.flush();
    std::cout << "[";
    for (int p = 0 ; p < BARWIDTH ; ++p){
        std::cout << " ";
    }
    std::cout << "] " << 0 << "% Reading file '" << curr_fname << "'" << "\r";
    std::cout.flush();
}

void ProgressUI::UpdateReadProgress(const size_t &curr_pos)
{
    if ( curr_pos/double(length) - shown*0.02 > 0 ){
        ++shown;
        std::cout << "                                                                                            " << "\r";
        std::cout.flush();
        std::cout << "[";
        int pos = int( BARWIDTH * curr_pos / double(length) );
        for (int p = 0 ; p < BARWIDTH ; ++p){
            if ( p < pos ) std::cout << "=";
            else if ( p== pos ) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(100 * (curr_pos / double(length))) << "% Reading file '" << curr_fname << "'\r";
        std::cout.flush();
    }
}

void ProgressUI::StartBuildingEvents(const size_t &size)
{
    length = size;
    shown = 0;
    std::cout << "                                                                                            " << "\r";
    std::cout.flush();
    std::cout << "[";
    for (int p = 0 ; p < BARWIDTH ; ++p){
        std::cout << " ";
    }
    std::cout << "] " << 0 << "% Building events file '" << curr_fname << "'" << "\r";
    std::cout.flush();
}

void ProgressUI::UpdateEventBuildingProgress(const size_t &curr_pos)
{
    if ( curr_pos/double(length) - shown*0.02 > 0 ){
        ++shown;
        std::cout << "                                                                                            " << "\r";
        std::cout.flush();
        std::cout << "[";
        int pos = int( BARWIDTH * curr_pos / double(length) );
        for (int p = 0 ; p < BARWIDTH ; ++p){
            if ( p < pos ) std::cout << "=";
            else if ( p== pos ) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(100 * (curr_pos / double(length))) << "% Building events, file '" << curr_fname << "'\r";
        std::cout.flush();
    }
}

void ProgressUI::StartFillingHistograms(const size_t &size)
{
    length = size;
    shown = 0;
    std::cout << "                                                                                            " << "\r";
    std::cout.flush();
    std::cout << "[";
    for (int p = 0 ; p < BARWIDTH ; ++p){
        std::cout << " ";
    }
    std::cout << "] " << 0 << "% Filling histograms, file '" << curr_fname << "'" << "\r";
    std::cout.flush();
}

void ProgressUI::UpdateHistFillProgress(const size_t &curr_pos)
{
    if ( curr_pos/double(length) - shown*0.02 > 0 ){
        ++shown;
        std::cout << "                                                                                            " << "\r";
        std::cout.flush();
        std::cout << "[";
        int pos = int( BARWIDTH * curr_pos / double(length) );
        for (int p = 0 ; p < BARWIDTH ; ++p){
            if ( p < pos ) std::cout << "=";
            else if ( p== pos ) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(100 * (curr_pos / double(length))) << "% Filling histograms, file '" << curr_fname << "'\r";
        std::cout.flush();
    }
}

void ProgressUI::StartFillingTree(const size_t &size)
{
    length = size;
    shown = 0;
    std::cout << "                                                                                            " << "\r";
    std::cout.flush();
    std::cout << "[";
    for (int p = 0 ; p < BARWIDTH ; ++p){
        std::cout << " ";
    }
    std::cout << "] " << 0 << "% Filling trees, file '" << curr_fname << "'" << "\r";
    std::cout.flush();
}

void ProgressUI::UpdateTreeFillProgress(const size_t &curr_pos)
{
    if (curr_pos / double(length) - shown * 0.02 > 0) {
        ++shown;
        std::cout << "                                                                                            " << "\r";
        std::cout.flush();
        std::cout << "[";
        int pos = int(BARWIDTH * curr_pos / double(length));
        for (int p = 0; p < BARWIDTH; ++p) {
            if (p < pos) std::cout << "=";
            else if (p == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(100 * (curr_pos / double(length))) << "% Filling trees, file '" << curr_fname << "'\r";
        std::cout.flush();
    }
}

void ProgressUI::Finish()
{
    std::cout << "[";
    for (int p = 0 ; p < BARWIDTH ; ++p){
        std::cout << "=";
    }
    std::cout << "] " << "100% Done processing file '" << curr_fname << std::endl;
}