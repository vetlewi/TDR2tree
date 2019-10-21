#ifndef BUFFERTYPE_H
#define BUFFERTYPE_H

#include <cstdint>

//! Class to store a general buffer.
class Buffer {
protected:

	/*!
	 * Constructor
	 * \param sz Size of buffer in number of bytes.
	 * \param buf Buffer data.
	 */
	Buffer(int sz, char *buf) : size( sz ), buffer( buf ){}

    //! Private implementation of the resize method.
    void pResize(unsigned int new_size /*!< New size of the buffer(number of elements). */)
    {
        if ( buffer )
            delete GetBuffer();
        size = new_size;
        buffer = new char[size];
    }

public:
	//! Virtual no-op destructor.
	virtual ~Buffer() = default;

	//! Get data from the buffer.
	/*! \return the dataword from the buffer.
	 */
	char GetData(int idx) const
		{ return buffer[idx]; }

	//! Overloaded [] operator.
	/*! \return GetData(idx)
	 */
	char operator[](int idx) const
		{ return GetData(idx); }

	//! Give access to the data.
	/*! Used by classes to fetch the data.
	 */
	char* GetBuffer()
		{ return buffer; }

    //! Give const access to the data.
    char* CGetBuffer() const
        { return buffer; }

	//! Get the buffer size.
	/*! \return the number of elements in the buffer.
	 */
	virtual unsigned int GetSize() const
		{ return size; }

	//! Create a new buffer of the same type.
	virtual Buffer* New() { return nullptr; }

	//! Resize the buffer.
	virtual void Resize(const int &new_size){ pResize(new_size); }

protected:
	//! Size of the buffer in number of elements.
	unsigned int size;

	//! The buffer data.
	char *buffer;
};

class SiriusBuffer : public Buffer {
    enum { BUFSIZE = 0x8000 /*!< The size of the buffer in number of words. */};
public:
    SiriusBuffer(int sz, char *buf ) : Buffer(sz, buf) { }
    SiriusBuffer() : Buffer(BUFSIZE, new char[BUFSIZE]) { }
    ~SiriusBuffer() override { delete GetBuffer(); }
    SiriusBuffer* New() override { return new SiriusBuffer(); }
};

class TDRBuffer : public Buffer {
	enum { BUFSIZE = 8200*sizeof(word_t) /*!< The size of the buffer in number of words. */};
public:
	TDRBuffer(int sz, word_t *buf ) : Buffer(sz, reinterpret_cast<char*>(buf)) { }
	TDRBuffer() : Buffer(BUFSIZE, new char[BUFSIZE]) { }
	~TDRBuffer() override { delete GetBuffer(); }
	TDRBuffer* New() override { return new TDRBuffer(); }

	void Resize(const int &new_size) override{ pResize(new_size*sizeof(word_t)); }
	unsigned int GetSize() const override { return size/sizeof(word_t); }
};

#endif // BUFFERTYPE_H