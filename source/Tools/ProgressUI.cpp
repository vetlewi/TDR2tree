//
// Created by Vetle Wegner Ingeberg on 2019-05-28.
//

#include "ProgressUI.h"

#include <iostream>

#include <indicators/cursor_control.hpp>

std::string StripSlash(const std::string &str)
{
    if ( str.back() == '/' )
        return StripSlash(str.substr(0, str.size()-1));
    auto begin = str.find_last_of('/');
    if ( begin == std::string::npos )
        return str;
    else
        return str.substr(begin+1);
}

ProgressBar::ProgressBar(const std::string &_filename, const size_t &size)
    : indicators::BlockProgressBar{ indicators::option::BarWidth(80),
                                    indicators::option::ForegroundColor{indicators::Color::red},
                                    indicators::option::ShowElapsedTime{true},
                                    indicators::option::ShowRemainingTime{true},
                                    indicators::option::Stream{std::cout}}
    , filename( StripSlash(_filename) )
    , length( size )
    , shown( 0 )
{
    // We only want the part after the last '/' character
    auto begin = _filename.find_last_of('/');
    filename = ( begin == std::string::npos ) ? _filename : _filename.substr(begin+1);
    set_option(indicators::option::PrefixText(filename + ": Reading  "));
    //bar.push_back(*this);
}

void ProgressBar::UpdateProgress(const size_t &pos)
{
    if ( pos/double(length) - shown * 0.01 > 0 ){
        ++shown;
        set_progress(100*(pos/float(length)));
    }
}

void ProgressBar::FinishProgress()
{
    set_option(indicators::option::PrefixText(filename + ": Complete "));
    set_option(indicators::option::ForegroundColor{indicators::Color::green});
    set_progress(100);
    mark_as_completed();
}

SpinnerBar::SpinnerBar(const std::string &fname)
    : indicators::ProgressSpinner{indicators::option::ForegroundColor{indicators::Color::yellow},
                                  indicators::option::ShowPercentage{false},
                                  indicators::option::FontStyles{std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}}
    , filename( StripSlash(fname) )
{
    set_option(indicators::option::PostfixText{"Writing to file '" + filename + "'"});
    runner = std::thread(&SpinnerBar::run, this);
}

void SpinnerBar::run()
{
    while ( !done ){
        tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    set_option(indicators::option::ShowSpinner{false});
    set_option(indicators::option::ForegroundColor{indicators::Color::green});
    set_option(indicators::option::PrefixText{"✔"});
    set_option(indicators::option::PostfixText{"Writing to file '" + filename + "'"});
    mark_as_completed();
}

void SpinnerBar::Finish()
{
    done = true;
    if ( runner.joinable() ){
        runner.join();
    }
}

ProgressUI::ProgressUI()
{
    indicators::show_console_cursor(false);
}

ProgressUI::~ProgressUI()
{
    indicators::show_console_cursor(true);
}