//
// Created by Vetle Wegner Ingeberg on 2019-05-28.
//

#ifndef TDR2TREE_PROGRESSUI_H
#define TDR2TREE_PROGRESSUI_H

#include <string>
#include <thread>
#include <atomic>
#include <indicators/progress_spinner.hpp>
#include <indicators/block_progress_bar.hpp>


class ProgressBar : public indicators::BlockProgressBar
{
private:
    // We keep information about the file name and the total size
    std::string filename;
    size_t length;
    size_t shown;

public:
    ProgressBar(const std::string &filename, const size_t &size);

    void UpdateProgress(const size_t &pos);

    void FinishProgress();

};

class SpinnerBar : public indicators::ProgressSpinner
{
private:
    std::string filename;
    std::atomic<bool> done{false};
    std::thread runner;
    void run();
public:
    SpinnerBar(const std::string &fname);
    void Finish();

};

class ProgressUI {

public:

    //! Default ctor.
    ProgressUI();
    ~ProgressUI();

    //! Give the user feedback that we are starting the readout of a new file.
    ProgressBar StartNewFile(const std::string &fname, const size_t &flength){ return ProgressBar(fname, flength); }

    //! Notify the user that we are done sorting the file.
    SpinnerBar FinishSort(const std::string &fname){ return SpinnerBar(fname); }
};


#endif //TDR2TREE_PROGRESSUI_H
