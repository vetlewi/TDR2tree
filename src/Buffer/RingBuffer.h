//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

namespace Fetcher {
    namespace Details {

        /*!
         * A ring buffer.
         * \brief The implementation is messy.
         */
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

// ########################################################################

        template<class T, int N, bool created>
        RingBuffer<T,N,created>::RingBuffer()
                : head(0), tail(0), size(0)
        {
            for(int i=0; i<N; ++i)
                ring[i] = 0;
        }

// ########################################################################

        template<class T, int N, bool created>
        RingBuffer<T,N,created>::~RingBuffer()
        {
            if( created ) {
                for(int i=0; i<N; ++i)
                    delete ring[i];
            }
        }

// ########################################################################

        template<class T, int N, bool created>
        T* RingBuffer<T,N,created>::Put(T* t)
        {
            if( Full() )
                return 0;
            if( created ) {
                t = ring[head];
            } else {
                ring[head] = t;
            }
            head = (head+1)%N;
            size += 1;
            return t;
        }

// ########################################################################

        template<class T, int N, bool created>
        T* RingBuffer<T,N,created>::Get()
        {
            if( Empty() )
                return 0;
            T* r = created ? 0 : ring[tail];
            tail = (tail+1)%N;
            size -= 1;
            return r;
        }

// ########################################################################

        template<class T, int N, bool created>
        T* RingBuffer<T,N,created>::dummy = 0;
    }
}

#endif // RINGBUFFER_H
