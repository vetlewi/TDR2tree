//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "BasicStruct.h"
#include "experimentsetup.h"
#include "TDRParser.h"
#include "TDRTypes.h"
#include "XIA_CFD.h"
#include "Calibration.h"
#include "MemoryMap.h"
#include "ProgressUI.h"

#include <vector>
#include <iostream>

ProgressUI progress;

inline int64_t TSFactor(const ADCSamplingFreq &freq)
{
    if ( freq == f250MHz )
        return 8;
    else
        return 10;
}

std::vector<word_t> TDRtoWord(const std::vector<TDR::Entry_t> &entries)
{
    const DetectorInfo_t *dinfo;
    std::vector<word_t> words;
    words.reserve(entries.size());
    word_t word{};
    unsigned short adc_data;
    XIA::XIA_CFD_t cfd_res;

    for ( auto &entry : entries ){
        // This is also the place where we will remove any events with adc larger than 16384.
        adc_data = entry.adc->ADC_data;
        word = {entry.GetAddress(),
                adc_data,
                uint16_t(entry.tdc->ADC_data),
                entry.adc->fail,
                entry.adc->veto || (( adc_data & 0x8000 ) == 0x8000),
                entry.timestamp(),
                0,
                true,
                0};
        dinfo = GetDetectorPtr(word.address);
        cfd_res = XIA::XIA_CFD_Decode(dinfo->sfreq, word.cfddata);
        word.cfdcorr = cfd_res.first;
        word.cfdfail = cfd_res.second;

        word.energy = CalibrateEnergy(word);
        word.cfdcorr += CalibrateTime(word);

        word.timestamp *= TSFactor(dinfo->sfreq);
        word.timestamp += int64_t(word.cfdcorr);
        word.cfdcorr -= int64_t(word.cfdcorr);
        words.push_back(word);
    }
    return words;
}

class SingleThread {
private:
    const size_t size;
    std::vector<word_t> buffer;

    TDR::Parser parser;

public:

    SingleThread(const size_t &buffer_size = 196608) : size( buffer_size ){ buffer.reserve( size*2 ); }

    void Run(IO::MemoryMap &map, const bool &last)
    {
        auto *end = map.GetPtr() + map.GetSize();
        auto *header = TDR::FindHeader(map.GetPtr(), end);
        while ( header < end ){
            auto *next_header = TDR::FindHeader(header+reinterpret_cast<const TDR::TDR_header_t *>(header)->header_dataLen, end);
            auto data = TDRtoWord(parser.ParseBuffer(header, (next_header == end)&last));
            buffer.insert(buffer.end(), data.begin(), data.end());
            std::sort(buffer.begin(), buffer.end(), [](const word_t &lhs, const word_t &rhs)
            { return (double(rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });

            if ( buffer.size() > size ){
                std::vector out(buffer.begin(), buffer.begin()+buffer.size()-size);
                buffer.erase(buffer.begin(), buffer.begin()+buffer.size()-size);
            }
            header = next_header;
            progress.UpdateReadProgress(header - map.GetPtr());
        }
        progress.Finish();
    }
};

int main()
{
    IO::MemoryMap mfile("/Volumes/PR282/PR282/R94_0");
    progress.StartNewFile("R94_0", mfile.GetSize());

    SingleThread worker;
    worker.Run(mfile, true);
    return 0;
}