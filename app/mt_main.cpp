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

#include <readerwritercircularbuffer.h>

#include <thread>

#include "Reader.h"

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

class Task {
protected:
    std::atomic<bool> done = false;
public:
    virtual ~Task() = default;
    void Finish(){ done = true; }
};

class ReaderThread : public Task
{
public:
    using ReaderQueue = moodycamel::BlockingReaderWriterCircularBuffer<std::vector<TDR::Entry_t>>;
private:
    TDR::Parser parser;
    ReaderQueue queue;

public:

    ReaderThread(const size_t &cap) : queue( cap ){}

    void Run(IO::MemoryMap &map, const bool &last)
    {
        auto *end = map.GetPtr() + map.GetSize();
        auto *header = TDR::FindHeader(map.GetPtr(), end);
        while ( header < end ){
            auto *next_header = TDR::FindHeader(header+reinterpret_cast<const TDR::TDR_header_t *>(header)->header_dataLen, end);
            queue.wait_enqueue(parser.ParseBuffer(header, (next_header == end)&last));
            header = next_header;
            progress.UpdateReadProgress(header - map.GetPtr());
        }
    }

    ReaderQueue &GetQueue(){ return queue; }
};

class ConverterThread : public Task
{
public:
    using DecoderQueue = moodycamel::BlockingReaderWriterCircularBuffer<std::vector<word_t>>;

private:
    ReaderThread::ReaderQueue &input_queue;
    DecoderQueue output_queue;

public:

    ConverterThread(ReaderThread::ReaderQueue &input, const size_t &cap)
        : input_queue( input ), output_queue( cap ){}

    void Run()
    {
        std::vector<TDR::Entry_t> input;
        while ( true ){
            if ( input_queue.wait_dequeue_timed(input, std::chrono::milliseconds(10)) ){
                output_queue.try_enqueue(TDRtoWord(input));
            } else if ( done )
                break;
            else
                std::this_thread::yield();
        }
    }

    DecoderQueue &GetQueue(){ return output_queue; }

};

class BufferThread : public Task
{
public:
    using BufferQueue = moodycamel::BlockingReaderWriterCircularBuffer<std::vector<word_t>>; // This could be a MPMC since we only read some at a time
private:
    const size_t size;
    std::vector<word_t> buffer;

    ConverterThread::DecoderQueue &input_queue;
    BufferQueue output_queue;

public:

    BufferThread(ConverterThread::DecoderQueue &input, const size_t &cap, const size_t &size = 196608)
        : size( size ), input_queue( input ), output_queue( cap ){ buffer.reserve(2*size); }

    void Run()
    {
        std::vector<word_t> input;
        while ( true ){
            // First we will check if there is enough data to push to queue
            if ( buffer.size() > size ){
                // We push everything that we can
                output_queue.try_enqueue(std::vector(buffer.begin(), buffer.begin()+buffer.size()-size));
                buffer.erase(buffer.begin(), buffer.begin()+buffer.size()-size);
            }

            if ( input_queue.wait_dequeue_timed(input, std::chrono::milliseconds(10)) ){
                buffer.insert(buffer.end(), input.begin(), input.end());
                std::sort(buffer.begin(), buffer.end(), [](const word_t &lhs, const word_t &rhs)
                { return (double(rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });
            } else if ( done ){
                output_queue.wait_enqueue(std::move(buffer));
                break;
            } else {
                std::this_thread::yield();
            }
        }
    }

    BufferQueue &GetQueue(){ return output_queue; }
};

struct time_val_t {
    int64_t timestamp;
    double cfd_corr;

    /*time_val_t() = default;
    time_val_t(const int64_t &ts, const double &cfd) : timestamp( ts ), cfd_corr( cfd ){}
    time_val_t(const word_t &w) : timestamp( w.timestamp ), cfd_corr( w.cfdcorr ){}*/
};

double operator-(const time_val_t &lhs, const time_val_t &rhs){
    return double( lhs.timestamp - rhs.timestamp ) + lhs.cfd_corr - rhs.cfd_corr;
}

class SplitterThread : public Task
{
public:
    using SplitterQueue = moodycamel::BlockingReaderWriterCircularBuffer<std::vector<word_t>>;
private:

    const double gap_time;
    BufferThread::BufferQueue &input_queue;
    SplitterQueue output_queue;

    std::vector<word_t> buffer;

    void SplitEntries(){
        auto begin = buffer.begin();
        time_val_t pre_time = {begin->timestamp, begin->cfdcorr};
        while ( true ){
            auto end = std::find_if_not(begin, buffer.end(), [&pre_time, this](const word_t &evt){
                time_val_t time = {evt.timestamp, evt.cfdcorr};
                double timediff = time - pre_time;
                pre_time = time;
                return timediff < this->gap_time;
            });
            if ( end == buffer.end() )
                break;

            output_queue.try_enqueue(std::vector(begin, end));
            begin = end;
        }
        // Then we will remove everything up to begin (but not including)
        buffer.erase(buffer.begin(), begin);
    }

public:
    SplitterThread(BufferThread::BufferQueue &input, const double &gap, const size_t &cap)
        : gap_time( gap ), input_queue( input ), output_queue( cap ) {}

    SplitterQueue &GetQueue(){ return output_queue; }

    void Run()
    {
        std::vector<word_t> input;
        while ( true ){

            if ( input_queue.wait_dequeue_timed(input, std::chrono::milliseconds(10)) ){
                buffer.insert(buffer.end(), input.begin(), input.end());
                std::sort(buffer.begin(), buffer.end(), [](const word_t &lhs, const word_t &rhs)
                { return (double(rhs.timestamp - lhs.timestamp) + (rhs.cfdcorr - lhs.cfdcorr)) > 0; });
                SplitEntries();
            } else if ( done ){
                break;
            } else {
                std::this_thread::yield();
            }
        }
    }

};

template<class Queue_t, class T>
class Popper : public Task // Class that just pops entries of a queue.
{
private:
    Queue_t &input_queue;
public:

    Popper(Queue_t &input) : input_queue( input ){}

    void Run()
    {
        T input;
        while ( true ){
            if ( input_queue.wait_dequeue_timed(input, std::chrono::milliseconds(10)) ){
                // Do nothing...
            } else if ( done ){
                break;
            } else {
                std::this_thread::yield();
            }
        }
    }
};


int main(int argc, char *argv[])
{
    IO::MemoryMap mfile("/Volumes/PR282/PR282/R91_0");

    size_t cap = 256;
    if ( argc == 2 ){
        cap = std::stoi(argv[1]);
    }

    std::cout << "Cap: " << cap << std::endl;
    progress.StartNewFile("R94_0", mfile.GetSize());

    // Declare the Reader thread object
    ReaderThread reader(cap);
    ConverterThread converter(reader.GetQueue(), cap);
    BufferThread buffer(converter.GetQueue(), cap);
    SplitterThread splitter(buffer.GetQueue(), 1500., 2048);
    using Popper_t = Popper<moodycamel::BlockingReaderWriterCircularBuffer<std::vector<word_t>>, std::vector<word_t>>;
    Popper_t popper(splitter.GetQueue());

    // First we will start a thread for each of the objects.
    std::vector<std::pair<std::thread, Task *>> threads;
    threads.emplace_back(std::thread(&ReaderThread::Run, &reader, std::ref(mfile), true), &reader);
    threads.emplace_back(std::thread(&ConverterThread::Run, &converter), &converter);
    threads.emplace_back(std::thread(&BufferThread::Run, &buffer), &buffer);
    threads.emplace_back(std::thread(&SplitterThread::Run, &splitter), &splitter);
    threads.emplace_back(std::thread(&Popper_t::Run, &popper), &popper);

    // Now we just wait for everything to finish running
    for ( auto &runner : threads ){
        runner.second->Finish();
        if ( runner.first.joinable() ){
            runner.first.join(); // Blocking until task is finished
        }
    }
    return 0;
}