// FileReader 2.0

#include "FileReader.h"
#include "experimentsetup.h"
#include "Calibration.h"
#include "XIA_CFD.h"
#include "ProgressUI.h"

#include "BufferType.h"

#include <cstdint>
#include <algorithm>
#include <iostream>

extern ProgressUI progress;

struct DATA_HEADER_T {
        char     header_id[8];          /*!< Contains the string  EBYEDATA. (8 bytes) (byte 0-7) */
        uint32_t header_sequence;       /*!< Within the file. (4 bytes) (8-11) */
        uint16_t  header_stream;        /*!< Data acquisition stream number (in the range 1=>4). (2 bytes) (12-13) */
        uint16_t  header_tape;          /*!< =1. (2 bytes) (14-15) */
        uint16_t  header_MyEndian;      /*!< Written as a native 1 by the tape server (2 bytes) (16-17) */
        uint16_t  header_DataEndian;    /*!< Written as a native 1 in the hardware structure of the data following. (2 bytes) (18-19) */
        uint32_t header_dataLen;        /*!< Total length of useful data following the header in bytes. (4 bytes) (20-23) */
    };

#define BLOCKSIZE 65536
static uint64_t data_buffer[(BLOCKSIZE - 24)/sizeof(uint64_t)];
static word_t unpacked_buffer[(BLOCKSIZE - 24)/sizeof(uint64_t)];
static int found, read;
static int64_t topTime;
static struct DATA_HEADER_T header;

inline uint64_t extract(const uint64_t &val, const int &begin, const int &end)
{
    uint64_t mask = (1 << (end - begin)) - 1;
    return (val >> begin) & mask;
}

// #########################################################

int DecodeWords(word_t *data, uint64_t *buf, size_t length)
{
    int n_decoded=0;

    // We declare them here so that we don't use clock cycles doing that in the loop.
    uint16_t adcdata_w1;
    uint16_t adcdata_w2;
    uint16_t address_w1;
    uint16_t address_w2;
    uint64_t timestamp_w1;
    uint64_t timestamp_w2;

    if (topTime == 0){
        for (size_t i = 0 ; i < length ; ++i){
            uint16_t iden = extract(buf[i], 62, 64);
            if (iden == 2){
                topTime = extract(buf[i], 32,32+20)<<28;
                break;
            }
        }
    }

    for (size_t i = 0 ; i < length - 1 ; ++i){
        uint16_t iden = extract(buf[i], 62, 64);
        if (iden == 2){
            topTime = extract(buf[i], 32, 32+20) << 28;
        } else if (iden == 3) { // We are assuming that two consecutive words are the ADC value and the CFD value.
            adcdata_w1 = extract(buf[i], 32, 32+16);
            adcdata_w2 = extract(buf[i+1], 32, 32+16);
            address_w1 = extract(buf[i], 32+16, 32+16+12);
            address_w2 = extract(buf[i+1],32+16, 32+16+12);
            timestamp_w1 = extract(buf[i], 0, 28)+topTime;
            timestamp_w2 = extract(buf[i+1], 0, 28)+topTime;
            if ( ((address_w1&16)==0) && (address_w2 == address_w1+16) && (timestamp_w1 == timestamp_w2) ){
                data[n_decoded].address = address_w1;
                data[n_decoded].adcdata = adcdata_w1;
                data[n_decoded].cfddata = adcdata_w2;
                data[n_decoded].timestamp = timestamp_w1;
                if ( data[n_decoded].adcdata < 16384 ) // ADC data should never be larger than 16484. If, then COMPTON suppressed.
                    ++n_decoded;
                ++i;
            } else if ( ((address_w2&16)==0) && (address_w1 == address_w2+16) && (timestamp_w2 == timestamp_w1) ){
                data[n_decoded].address = address_w2;
                data[n_decoded].adcdata = adcdata_w2;
                data[n_decoded].cfddata = adcdata_w1;
                data[n_decoded].timestamp = timestamp_w2;
                if ( data[n_decoded].adcdata < 16384 ) // ADC data should never be larger than 16484. If, then COMPTON suppressed.
                    ++n_decoded;
                ++i;
            }
        }
    }
    DetectorInfo_t dinfo;
    for (int i = 0 ; i < n_decoded ; ++i){
        switch ( GetSamplingFrequency(data[i].address) ) {
        case f100MHz :
            data[i].cfdcorr = XIA_CFD_Fraction_100MHz(data[i].cfddata, &data[i].cfdfail);
            data[i].timestamp *= 10;
            if ( data[i].cfddata == 0 )
                data[i].cfdfail = 1;
            break;
        case f250MHz :
            data[i].cfdcorr = XIA_CFD_Fraction_250MHz(data[i].cfddata, &data[i].cfdfail);
            data[i].timestamp *= 8;
            if ( data[i].cfddata == 0 )
                data[i].cfdfail = 1;
            break;
        case f500MHz :
            data[i].cfdcorr = XIA_CFD_Fraction_500MHz(data[i].cfddata, &data[i].cfdfail);
            data[i].timestamp *= 10;
            if ( data[i].cfddata == 0 )
                data[i].cfdfail = 1;
            break;
        default :
            data[i].cfdcorr = 0;
            data[i].cfdfail = 1;
            data[i].timestamp *= 10;
            break;
        }


        data[i].cfdcorr += CalibrateTime(data[i]);

    }

    return n_decoded;
}

// #########################################################

bool ReadBlock(std::FILE *file_stdio)
{
    // Check if file is open.
    if ( !file_stdio )
        return false;

    // Read header
    if ( std::fread(&header, sizeof(DATA_HEADER_T), 1, file_stdio) != 1)
        return false; // Unable to read the header :'(

    // Read the data block.
    if ( std::fread(data_buffer, sizeof(uint64_t), (BLOCKSIZE - 24)/sizeof(uint64_t), file_stdio) != (BLOCKSIZE - 24)/sizeof(uint64_t))
        return false; // We did not get the data we expected :'(

    // Decode the block of data!
    found = DecodeWords(unpacked_buffer, data_buffer, (BLOCKSIZE - 24)/sizeof(uint64_t));
    read = 0;
    return true;
}

// #########################################################

bool seek(std::FILE *file_stdio, int num_block)
{
    if (num_block == 0)
        return ReadBlock(file_stdio);

    int block_read=0;
    while (block_read < num_block){
        if ( !ReadBlock(file_stdio) )
            return false;
        ++block_read;
    }

    return true;
}

// #########################################################

FileReader::FileReader() :
    file_stdio( nullptr ),
    errorflag( false )
{
}

// #########################################################

FileReader::~FileReader()
{
	Close();
}

// #########################################################

bool FileReader::Open(const char *filename, int want)
{
	Close();
	file_stdio = std::fopen(filename, "rb");

	if ( !file_stdio ){
	    std::cerr << __PRETTY_FUNCTION__ << ": Unable to open file '" << filename << "'" << std::endl;
	    exit(EXIT_FAILURE);
	}

    fseek(file_stdio, 0, SEEK_END);
    auto size = size_t(ftell(file_stdio));
    fseek(file_stdio, 0, SEEK_SET);
    progress.StartNewFile(filename, size);

    // We open a new file... Make sure 'topTime' is zero!
    topTime = 0;

    errorflag = ( file_stdio==nullptr )
                || ( !seek(file_stdio, want) );


    return !errorflag;
}

// #########################################################

void FileReader::Close()
{
    if (file_stdio){
        std::fclose( file_stdio );
        file_stdio = nullptr;
	}
}

// #########################################################

int FileReader::Read(word_t *buffer, int size)
{

    if ( errorflag || (!file_stdio) ){
        return -1;
    }


    int having = 0;
    while ( having < size ){
        if ( !ReadEvent( buffer[having++]) ){

            // Check if EOF or error.
            if ( feof(file_stdio) ){
                errorflag = false;
                Close();
            } else if ( ferror(file_stdio) ){
                errorflag = true;
            } else {
                errorflag = true;
            }
            return errorflag ? -1 : having;
        }
    }
	return having;
}

// #########################################################

int FileReader::Read(TDRBuffer *buffer)
{
    int found = Read(buffer->GetBuffer(), buffer->GetSize());
    if ( file_stdio )
        progress.UpdateReadProgress(ftell(file_stdio));
    return found;
}

// #########################################################

bool FileReader::ReadEvent(word_t &hit)
{

    // First we check if we have 'handed over' all the events extracted in the current
    if ( errorflag ) {
        return false;
    } else if (read < found){
        hit = unpacked_buffer[read++];
        return true;
    } else if ( ReadBlock(file_stdio) ) {
        if (read < found){
            hit = unpacked_buffer[read++];
            return true;
        } else
            return false;
    } else {
        return false;
    }
}

#define READ_NUM 65536

std::vector<word_t> FileReader::GetFile(const char *fname)
{
    FileReader freader;
    freader.Open(fname);
    std::vector<word_t> data;
    data.reserve(128876544);
    word_t entries[READ_NUM];
    int read;
    while ( true ){
        read = freader.Read(entries, READ_NUM);
        data.insert(data.end(), entries, entries+read);
        if ( read < READ_NUM ){
            break;
        }
        progress.UpdateReadProgress(ftell(freader.file_stdio));
    }
    std::sort(data.begin(), data.end(), [](const word_t &lhs, const word_t &rhs){ return ((lhs.timestamp - rhs.timestamp) + (lhs.cfdcorr - rhs.cfdcorr)) < 0; });

    return data;
}
