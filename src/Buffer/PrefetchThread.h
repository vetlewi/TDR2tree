//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#ifndef PREFETCHTHREAD_H
#define PREFETCHTHREAD_H

#include "PThreadMutex.h"
#include "RingBuffer.h"

namespace Fetcher {

    class Buffer;
    class FileReader;

    namespace Details {

        //! Class used by MTFileBufferFetcher to read buffers in a separate thread.
        class PrefetchThread
        {
        public:
            //! Initialize, but do not yet start running.
            PrefetchThread(FileReader *reader,     /*!< Helper to perform the actual file reading. */
                           Buffer *template_buffer /*!< Buffer object to be "multiplied". */);

            //! Cleanup after the thread stopped running.
            ~PrefetchThread();

            //! Start the new thread.
            void Start();

            //! Called to get a new buffer for sorting.
            Buffer *ReadingBegins();

            //! Called after sorting a buffer has finished.
            void ReadingEnds();

            //! Stop the thread.
            void Stop();

        private:
            //! The main loop of the thread.
            void StartReading();

            //! Helper for pthread_create.
            static void *Run(void *v)
            {
                ((PrefetchThread *) v)->StartReading();
                return nullptr;
            }

            //! The mutex for synchronizing access to the ring buffers.
            PThreadMutex mutex;

            //! The condition "write ring full".
            pthread_cond_t cond_full;

            //! The condition "read ring not empty".
            pthread_cond_t cond_avail;

            //! The thread object;
            pthread_t thread;

            //! The file reading implementation.
            FileReader *reader;

            enum {
                NBUFFERS = 8 /*!< By default, read up to 8 buffers in advance. */
            };

            //! The ring for fetching buffers.
            RingBuffer<Buffer, NBUFFERS, true> writeRing;

            //! The ring for reading buffers.
            RingBuffer<Buffer, NBUFFERS, false> readRing;

            //! Flag set to stop the thread. Only written by main thread.
            bool cancel;

            //! Flag that reading the file is finished. Only written by the prefetch thread.
            bool finished;
        };

    }
}

#endif // PREFETCHTHREAD_H
