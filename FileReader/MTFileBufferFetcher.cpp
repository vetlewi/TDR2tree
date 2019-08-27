/*******************************************************************************
 * Copyright (C) 2016 Vetle W. Ingeberg                                        *
 * Author: Vetle Wegner Ingeberg, v.w.ingeberg@fys.uio.no                      *
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

/*!
 * \file MTFileBufferFetcher.cpp
 * \brief Implementation of MTFileBufferFetcher, PThreadMutex, PThreadMutexLock, PrefetchThread and RingBuffer
 * \author Vetle W. Ingeberg
 * \author Alexander Bürger
 * \date 2010-2016
 * \copyright GNU Public License v. 3
 */

#include "MTFileBufferFetcher.h"

#include "FileReader.h"
#include "BufferType.h"
#include "aptr.ipp"

#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <stdint.h>

#define TEST_MTFILEBUFFERFETCHER 0

//! A helper class for handling pthread mutex objects.
class PThreadMutex {
public:
	//! Initilize the mutex.
	PThreadMutex()
		{ pthread_mutex_init( &mutex, 0 ); }

	//! Finalize the mutex.
	~PThreadMutex()
		{ pthread_mutex_destroy( &mutex ); }

	//! Lock the mutex.
	void Lock()
		{ pthread_mutex_lock( &mutex ); }

	//! Unlock the mutex.
	void Unlock()
		{ pthread_mutex_unlock( &mutex ); }

	//! Wait for condition.
	void Wait( pthread_cond_t *cond /*!< The condition to wait for. */)
		{ pthread_cond_wait( cond, &mutex ); }

private:
	//! The pthread mutex.
	pthread_mutex_t mutex;
};

//! A helper class for unlocking a mutex.
class PThreadMutexLock {
public:
	//! Lock the mutex.
	PThreadMutexLock(PThreadMutex& mtx /*!< The mutex to be locked. */)
		: mutex(mtx) { mutex.Lock(); }

	//! Unlock the mutex.
	~PThreadMutexLock()
		{ mutex.Unlock(); }

private:
	//! The mutex to be locked and unlocked.
	PThreadMutex& mutex;
};

//! A ring buffer.
template<class T, int N, bool created>
class RingBuffer {
public:
	RingBuffer();
	~RingBuffer();

	bool Full() const { return size==N; }
	bool Empty() const { return size==0; }
	T* Put(T* t=dummy);
	T* Get();
	void Set(unsigned int idx, T* t)
		{ ring[idx] = t; }

private:
	static T* dummy;

	T* ring[N];
	int head, tail, size;
};

template<class T, int N, bool created>
RingBuffer<T,N,created>::RingBuffer()
	: head(0), tail(0), size(0)
{
	for (int i = 0 ; i < N ; ++i)
		ring[i] = 0;
}

// ##############################################################

template<class T, int N, bool created>
RingBuffer<T,N,created>::~RingBuffer()
{
	if ( created ) {
		for (int i = 0 ; i < N ; ++i)
			delete ring[i];
	}
}

// ##############################################################

template<class T, int N, bool created>
T* RingBuffer<T,N,created>::Put(T* t)
{
	if ( Full() )
		return 0;
	if ( created ) {
		t = ring[head];
	} else {
		ring[head] = t;
	}
	head = (head+1)%N;
	size += 1;
	return t;
}

// ##############################################################

template<class T, int N, bool created>
T* RingBuffer<T,N,created>::Get()
{
	if ( Empty() )
		return 0;

	T* r = created ? 0 : ring[tail];
	tail = (tail+1)%N;
	size -= 1;
	return r;
}

// ##############################################################

template<class T, int N, bool created>
T* RingBuffer<T,N,created>::dummy = 0;

// ##############################################################
// ##############################################################



//! Class used by MTFileBufferFetcher to read buffers in a separate thread.
class PrefetchThread {
public:
	//! Initilize, but do not yet start running.
	PrefetchThread(FileReader* reader,		/*!< Helper to perform the actual file reading.	*/
                   TDRBuffer* template_buffer	/*!< Buffer object to be "multiplied". 			*/);

	//! Cleanup after the thread stopped running.
	~PrefetchThread();

	//! Start the new thread.
	void Start();

	//! Called to get a new buffer for sorting.
	TDRBuffer* ReadingBegins();

	//! Called after sorting a buffer has finished.
	void ReadingEnds();

	//! Stop the thread.
	void Stop();

private:
	//! The main loop of the thread.
	void StartReading();

	//! Helper for pthread_create.
	static void* Run(void* v)
		{ ((PrefetchThread*)v)->StartReading(); return 0; }

	//! The mutex for synchonizing access to the ring buffer.
	PThreadMutex mutex;

	//! The condition "write ring full".
	pthread_cond_t cond_full;

	//! The condition "read ring not empty"
	pthread_cond_t cond_avail;

	//! The thread object;
	pthread_t thread;

	//! The file reading implementation.
	FileReader* reader;

    enum { NBUFFERS = 128 /*!< By default, read up to 128 buffers in advance. */};
	
	//! The ring for fetching buffers.
	RingBuffer<TDRBuffer,NBUFFERS,true> writeRing;

	//! The ring for reading buffers.
	RingBuffer<TDRBuffer,NBUFFERS,false> readRing;

	//! Flag set to stop the thread. Only written by main thread.
	bool cancel;

	//! Flag that reading the file is finished. Only written by the prefetched thread.
	bool finished;	
};

PrefetchThread::PrefetchThread(FileReader* rdr, TDRBuffer* template_buffer)
	: reader( rdr )
	, cancel( false )
	, finished( false )
{
	pthread_cond_init( &cond_full,  0 );
	pthread_cond_init( &cond_avail, 0 );

	for (int i = 0 ; i < NBUFFERS ; ++i)
		writeRing.Set(i, template_buffer->New());
}

void PrefetchThread::Start()
{
	if ( pthread_create( &thread, NULL, PrefetchThread::Run, this) != 0){
		std::cerr << "Cannot create reader thread." << std::endl;
		exit(-1);
	}
}

void PrefetchThread::ReadingEnds()
{
	PThreadMutexLock lock( mutex );
	writeRing.Get();
	pthread_cond_signal( &cond_full );
}

TDRBuffer* PrefetchThread::ReadingBegins()
{
	PThreadMutexLock lock( mutex );
    while ( true ) {
		if ( finished && readRing.Empty() )
			return 0;
		if ( readRing.Empty() ){
			mutex.Wait( &cond_avail );
			continue;
		}
		return readRing.Get();
	}
}

void PrefetchThread::StartReading()
{
	while ( !cancel && !finished ){
		TDRBuffer* buffer = 0;
        { // Critical section
			PThreadMutexLock lock( mutex );
			while ( !cancel && !finished && writeRing.Full() )
				mutex.Wait( &cond_full );
			if ( cancel || finished )
				return;

			// Claim a buffer, but not yet say it is filled.
			buffer = writeRing.Put();
        } // Unlock in 'lock' destructor


        // Reading is time-consuming and should be performed while the
        // lock is released
        if ( reader->Read(buffer) <= 0 )
			finished = true;
        { // Critical section
			PThreadMutexLock lock( mutex );
			if ( !finished ) {
                // Mark the buffer as readable
				readRing.Put(buffer);
			} else {
				writeRing.Get();
			}
            // Tell main thread that data is available
			pthread_cond_signal( &cond_avail );
        } // Unlock in 'lock' destructor.
	}
}

// ##############################################################

void PrefetchThread::Stop()
{
    { // Critical section
		PThreadMutexLock lock( mutex );
		cancel = true;
		pthread_cond_signal( &cond_full );
    } // Unlock in 'lock' destructor.

    // Wait for the thread to terminate
	pthread_join( thread, NULL );
}

PrefetchThread::~PrefetchThread()
{
	pthread_cond_destroy( &cond_full );
	pthread_cond_destroy( &cond_avail );
}

// ##############################################################
// ##############################################################

MTFileBufferFetcher::MTFileBufferFetcher()
	: reader( new FileReader() )
	, template_buffer( new TDRBuffer() )
    , prefetch( 0 ) { }

MTFileBufferFetcher::~MTFileBufferFetcher()
{
	StopPrefetching();
}

const TDRBuffer* MTFileBufferFetcher::Next(Status& state)
{
	if ( reader->IsError() ){
		state = ERROR;
		return 0;
	}

	if ( !prefetch ){
		prefetch = new PrefetchThread( reader.get(), template_buffer.get() );
		prefetch->Start();
	} else {
		prefetch->ReadingEnds();
	}

    const TDRBuffer* b = prefetch->ReadingBegins();
    state = b ? OKAY : END;
	return b;
}

BufferFetcher::Status MTFileBufferFetcher::Open(const std::string& filename, int bufnum)
{
	StopPrefetching();
	int i = reader->Open( filename.c_str(), bufnum*template_buffer->GetSize()*32 );
    if (i > 0) return OKAY; else if ( i==0 ) return END; else return ERROR;
}

void MTFileBufferFetcher::StopPrefetching()
{
	if ( !prefetch )
		return;

	prefetch->Stop();
	delete prefetch;
	prefetch = 0;
}


#if TEST_MTFILEBUFFERFETCHER

int main()
{

	MTFileBufferFetcher fetcher;

	std::string filename = "R31_0";
	fetcher.Open(filename);
	int buffer_count = 0;
	while( true ){
		BufferFetcher::Status fstate;
		const WordBuffer* buf = fetcher.Next(fstate);
		if (fstate == BufferFetcher::END){
			std::cout << "The end has been reached." << std::endl;
			break;
		} else if ( fstate == BufferFetcher::ERROR ){
			std::cout << "We have an error!" << std::endl;
			break;
		}
		buffer_count += 1;
	}
	std::cout << "We have read " << buffer_count << " buffers." << std::endl;
	return 0;
}

#endif
