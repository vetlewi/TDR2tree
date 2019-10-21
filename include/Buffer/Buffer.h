/* -*- c++ -*-
 * Buffer.h
 *
 *  Created on: 10.03.2010
 *      Author: Alexander Bürger
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <stdexcept>

namespace Fetcher {

    //! An event buffer.
    class Buffer
    {
    protected:

        //! Initialize a buffer with a given size and data buffer.
        Buffer(unsigned int sz, char *bffr)
                : size(sz), buffer(bffr) {}

        void SetBuffer(char *bffr) { buffer = bffr; }


    public:
        //! Virtual no-op destructor.
        virtual ~Buffer(){};

        //! Get the size of the buffer in bytes.
        size_t GetSizeChar() const { return size; };

        //! Get the size of the buffer in the native buffer type.
        virtual size_t GetSize() const = 0;

        //! Get data from the buffer.
        /*! \return The data word from the buffer.
         */
        char GetData(int idx /*!< The index of the data word to retrieve. */) const { return buffer[idx]; }

        //! Convenience function for GetData().
        //int operator[](int idx) const { return GetData(idx); }

        //! Get access to the buffer memory.
        /*! This is to be used by classes fetching buffers,
         *  e.g. MTFileBufferFetcher.
         */
        char *GetBuffer() { return buffer; }

        char *GetBuffer() const { return buffer; }

        //! Create a new buffer of the same type.
        /*! \return a new buffer, or 0
         */
        virtual Buffer *New() = 0;

    private:
        //! The buffer size.
        size_t size;

        //! The buffer data.
        char *buffer;
    };

    template<typename T>
    class BufferType : public Buffer
    {

    protected:

        explicit BufferType(const size_t &size)
                : Buffer(size * sizeof(T) / sizeof(char), reinterpret_cast<char *>(new T[size])) {}

    public:

        size_t GetSize() const override { return GetSizeChar() * sizeof(char) / sizeof(T); }

        T operator[](const size_t &index) const
        {
            if (index >= GetSize()) {
                throw std::length_error("Buffer: Index out of bounds");
            }
            return reinterpret_cast<T>(GetData(index * sizeof(T) / sizeof(char)));
        }

        const T *GetRawData() const { return reinterpret_cast<const T *>(GetBuffer()); }

    public:

        ~BufferType() override { delete[] GetBuffer(); }

    };

// ########################################################################
// ########################################################################

    //! A sirius event buffer with 128kB size (32768 words).
    class SiriusBuffer : public BufferType<unsigned int>
    {
        enum
        {
            BUFSIZE = 0x8000 /*!< The size of a sirius buffer in words. */ };
    public:
        SiriusBuffer()
                : BufferType(BUFSIZE) {}

        Buffer *New() override { return new SiriusBuffer(); }
    };

    //! A TDR event buffer with 64kB size (8192 64-bit words).
    class TDRBuffer : public Fetcher::BufferType<uint64_t>
    {
        enum
        {
            BUFSIZE = 0x2000 /*!< The size of a sirius buffer in 32 */
        };

    public:
        TDRBuffer() : BufferType(BUFSIZE) {}

        Buffer *New() override { return new TDRBuffer(); }
    };

}

#endif /* BUFFER_H_ */