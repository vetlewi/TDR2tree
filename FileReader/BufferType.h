#ifndef BUFFERTYPE_H
#define BUFFERTYPE_H

#include <stdint.h>
#include "BasicStruct.h"

//! Class to store a general buffer.
template <typename T>
class Buffer {
protected:
	//! Initilizer
	Buffer(int sz	/*!< The size of the buffer in number of elements	*/
		  , T *buf	/*!< The buffer data.								*/)
		: size( sz )
		, buffer( buf )
		, is_set( true ) { }

public:
	//! Virtual no-op destructor.
	virtual ~Buffer() { }

	//! Get data from the buffer.
	/*! \return the dataword from the buffer.
	 */
	T GetData(int idx) const
		{ return buffer[idx]; }

	//! Overloaded [] operator.
	/*! \return GetData(idx)
	 */
	T operator[](int idx) const
		{ return GetData(idx); }

	//! Give access to the data.
	/*! Used by classes to fetch the data.
	 */
	T* GetBuffer()
		{ return buffer; }

    //! Give const access to the data.
    T* CGetBuffer() const
        { return buffer; }

	//! Get the buffer size.
	/*! \return the number of elements in the buffer.
	 */
	unsigned int GetSize() const
		{ return size; }

	//! Create a new buffer of the same type.
	virtual Buffer* New() { return 0; }

	//! Resize the buffer.
	void Resize(unsigned int new_size /*!< New size of the buffer(number of elements. */)
	{
		if ( is_set )
			delete GetBuffer();
		size = new_size;
		buffer = new T[size];
		is_set = true;
	}

private:
	//! Size of the buffer in number of elements.
	unsigned int size;

	//! The buffer data.
	T *buffer;

	//! If the buffer is created or not.
	bool is_set;
};

class TDRBuffer : public Buffer<word_t> {
	enum { BUFSIZE = 65536 /*!< The size of the buffer in number of words. */};
public:
	TDRBuffer(int sz, word_t *buf ) : Buffer<word_t>(sz, buf) { }
	TDRBuffer() : Buffer<word_t>(BUFSIZE, new word_t[BUFSIZE]) { }
	~TDRBuffer() { delete GetBuffer(); }
	TDRBuffer* New() { return new TDRBuffer(); }
};

#endif // BUFFERTYPE_H