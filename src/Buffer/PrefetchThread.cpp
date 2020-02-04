//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#include "Buffer/Buffer.h"
#include "Buffer/FileReader.h"

#include "PrefetchThread.h"

#include <iostream>

using namespace Fetcher::Details;

PrefetchThread::PrefetchThread(FileReader* rdr, Buffer* template_buffer)
        : reader( rdr )
        , cancel( false )
        , finished( false )
{
    pthread_cond_init( &cond_full,  nullptr );
    pthread_cond_init( &cond_avail, nullptr );

    for(int i=0; i<NBUFFERS; ++i)
        writeRing.Set(i, template_buffer->New());
}

// ########################################################################

void PrefetchThread::Start()
{
    if( pthread_create( &thread, nullptr, PrefetchThread::Run, this ) != 0 ) {
        std::cerr << "cannot create reader thread." << std::endl;
        exit( -1 );
    }
}

// ########################################################################

void PrefetchThread::ReadingEnds()
{
    PThreadMutexLock lock( mutex );
    writeRing.Get();
    pthread_cond_signal( &cond_full );
}

// ########################################################################

Fetcher::Buffer* PrefetchThread::ReadingBegins()
{
    PThreadMutexLock lock( mutex );
    while( true ) {
        if( finished && readRing.Empty() )
            return 0;
        if( readRing.Empty() ) {
            mutex.Wait( &cond_avail );
            continue;
        }
        return readRing.Get();
    }
}

// ########################################################################

void PrefetchThread::StartReading()
{
    while( !cancel && !finished ) {
        Buffer* buffer = nullptr;
        { // critical section
            PThreadMutexLock lock( mutex );
            while( !cancel && !finished && writeRing.Full() )
                mutex.Wait( &cond_full );
            if( cancel || finished )
                return;

            // claim a buffer, but do not yet say it is filled
            buffer = writeRing.Put();
        } // unlock in 'lock' destructor

        // reading is time-consuming and should be performed while the
        // lock is released
        if( reader->Read((char*)buffer->GetBuffer(), buffer->GetSize()*4 ) <= 0 )
            finished = true;

        { // critical section
            PThreadMutexLock lock( mutex );
            if( !finished ) {
                // mark the buffer as readable
                readRing.Put(buffer);
            } else {
                // would be wrong if > 1 thread; fortunately we have
                // only 1; for two threads T1 and T2, if T1 finishes
                // the previous critical section first, then T2 runs
                // that section, reads a buffer before T1, and detects
                // end of file, then T2 would "free" the buffer taken
                // by T1
                writeRing.Get();
            }
            // tell main thread that data are available
            pthread_cond_signal( &cond_avail );
        } // unlock in 'lock' destructor
    }
}

// ########################################################################

void PrefetchThread::Stop()
{
    { // critical section
        PThreadMutexLock lock( mutex );
        cancel = true;
        pthread_cond_signal( &cond_full );
    } // unlock in 'lock' destructor

    // wait for thread to terminate
    pthread_join( thread, nullptr);
}

// ########################################################################

PrefetchThread::~PrefetchThread()
{
    pthread_cond_destroy( &cond_full );
    pthread_cond_destroy( &cond_avail );
}