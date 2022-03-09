//
// Created by Vetle Wegner Ingeberg on 09/03/2022.
//

#include "XIAReader.h"
#include "XIA_CFD.h"
#include "MemoryMap.h"
#include "experimentsetup.h"
#include "Calibration.h"

using namespace Task;

struct XIA_base_v2 {
    unsigned address : 12;
    unsigned headerLen : 5;
    unsigned eventLen : 14;
    bool finishCode : 1;
    unsigned event_ts_lo : 32;
    unsigned event_ts_hi : 16;
    unsigned cfd_result : 16;
    unsigned event_energy : 16;
    unsigned traceLen : 15;
    bool traceOutOfRange : 1;
    explicit operator word_t();
};

inline int64_t TSFactor(const ADCSamplingFreq &freq)
{
    if ( freq == f250MHz )
        return 8;
    else
        return 10;
}

XIA_base_v2::operator word_t()
{
    int64_t timestamp = int64_t(event_ts_hi) << 32;
    timestamp |= event_ts_lo;
    timestamp *= TSFactor(GetDetectorPtr(address)->sfreq);

    auto cfd_res = XIA::XIA_CFD_Decode(GetDetectorPtr(address)->sfreq, cfd_result);

    word_t word = {uint16_t(address), uint16_t(event_energy), uint16_t(cfd_result),
                   finishCode, finishCode, timestamp, timestamp, 0, false, 0};

    word.energy = CalibrateEnergy(word);
    word.timestamp += CalibrateTime(word);
    return word;
}

XIAReader::XIAReader(const std::vector<std::string> &files, ProgressUI *_ui, const size_t &capacity, const size_t &_buffer_size)
    : file_names( files )
    , output_queue( capacity )
    , ui( _ui )
    , buffer_size( _buffer_size )
    , buffer( )
{
    buffer.reserve( buffer_size );
    for ( auto &file : files ){
        mapped_files.emplace_back(new IO::MemoryMap(file.c_str()));
    }
}
/*
const char *XIAReader::AddEntries(const char *begin, const char *end)
{
    // Convert to 32 bit words since that is the native size of XIA modules
    const auto *pos = reinterpret_cast<const uint32_t *>(begin);
    const auto *lend = reinterpret_cast<const uint32_t *>(end);
    buffer.clear();
    while ( pos < lend && buffer.size() < buffer_size ){
        auto entry = *reinterpret_cast<const XIA_base_v2 *>(pos);
        buffer.push_back(word_t(*entry));
    }
    return reinterpret_cast<const char *>(pos);
}

 */

void XIAReader::RunWithUI()
{
    int num = 0;
    for ( auto &file : mapped_files ){
        ProgressBar bar = ui->StartNewFile(file_names[num++], file->GetSize());
        auto *end = file->GetPtr() + file->GetSize(); // We operate in 32 bit words...
        auto *pos = file->GetPtr();


        bar.FinishProgress();
    }
}

void XIAReader::Run()
{
    if ( ui )
        RunWithUI();
    else
        RunWithoutUI();
}
