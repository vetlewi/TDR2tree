//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#ifndef PTHREADMUTEX_H
#define PTHREADMUTEX_H

#include <pthread.h>

namespace Fetcher {
    namespace Details {

        //! A helper class for handling pthread mutex objects.
        class PThreadMutex
        {
        public:
            //! Initialize the mutex.
            PThreadMutex() { pthread_mutex_init(&mutex, 0); }

            //! Finalize the mutex.
            ~PThreadMutex() { pthread_mutex_destroy(&mutex); }

            //! Lock the mutex.
            void Lock() { pthread_mutex_lock(&mutex); }

            //! Unlock the mutex.
            void Unlock() { pthread_mutex_unlock(&mutex); }

            //! Wait for a condition.
            void Wait(pthread_cond_t *cond /*!< The condition to wait for. */) { pthread_cond_wait(cond, &mutex); }

        private:
            // disabled, not implemented
            PThreadMutex(const PThreadMutex &other);

            PThreadMutex &operator=(const PThreadMutex &other);

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
            // disabled, not implemented
            PThreadMutexLock(const PThreadMutexLock& other);
            PThreadMutexLock& operator=(const PThreadMutexLock& other);

            //! The mutex to be locked and unlocked.
            PThreadMutex& mutex;
        };

    }
}

#endif // PTHREADMUTEX_H
