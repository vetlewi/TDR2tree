//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#include "Converter.h"

#include "experimentsetup.h"
#include "Calibration.h"
#include "XIA_CFD.h"
#include "BasicStruct.h"
#include "TDREntry.h"
#include "TDRTypes.h"

using namespace Task;

inline int64_t TSFactor(const ADCSamplingFreq &freq)
{
    if ( freq == f250MHz )
        return 8;
    else
        return 10;
}

std::vector<word_t> Converter::TDRtoWord(const std::vector<TDR::Entry_t> &entries)
{
    const DetectorInfo_t *dinfo;
    std::vector<word_t> words;
    words.reserve(entries.size());
    word_t word{};
    unsigned short adc_data;
    XIA::XIA_CFD_t cfd_res;

    for ( auto &entry : entries ){
        // This is also the place where we will remove any events with the top bit set
        // These have been veto'ed due to the Compton shield.
        adc_data = entry.adc->ADC_data;
        if ( vetoAction != VetoAction::ignore && ( adc_data & 0x8000 ) == 0x8000 ) {
            if ( vetoAction == VetoAction::remove ){
                continue;
            } else if ( vetoAction == VetoAction::keep ){
                adc_data &= 0x7FFF;
            }
        }
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
        //word.cfdcorr += CalibrateTime(word);
        word.timestamp *= TSFactor(dinfo->sfreq);
        word.timestamp_raw = word.timestamp;
        word.timestamp = word.timestamp + CalibrateTime(word);

        words.push_back(word);
    }
    return words;
}

void Converter::Run()
{
    std::vector<TDR::Entry_t> input;
    while ( true ){
#ifndef USE_ATOMIC_QUEUE
        if ( input_queue.wait_dequeue_timed(input, std::chrono::microseconds(100)) ){
            output_queue.wait_enqueue(TDRtoWord(input));
        } else if ( done ){
            break;
        }
#else
        if ( input_queue.was_empty() && done )
            break;
        else {
            input = input_queue.pop();
            //output_queue.wait_enqueue(TDRtoWord(input));
            output_queue.push(TDRtoWord(input));
        }
#endif // USE_ATOMIC_QUEUE
    }
}