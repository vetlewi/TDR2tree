/* -*- c++ -*-
 *
 *  Created on: 10.03.2010
 *      Author: Alexander Bürger
 */

#ifndef HISTOGRAM1D_H_
#define HISTOGRAM1D_H_

#include <Histograms.h>
#include <vector>

//#define H1D_USE_BUFFER 1

// ########################################################################

//! A one-dimensional histogram.
class Histogram1D : public Named {
public:
  //! The type used to count in each bin.
  typedef size_t data_t;

  //! Typedef if buffer is used.
    struct buf_t {
        Axis::bin_t x;
        data_t w;
        buf_t(Axis::bin_t xx, data_t ww) : x(xx), w(ww) { }
    };
    typedef std::vector<buf_t> buffer_t;

  //! Construct a 1D histogram.
  Histogram1D( const std::string& name,   /*!< The name of the new histogram. */
               const std::string& title,  /*!< The title of teh new histogram. */
               Axis::index_t channels,    /*!< The number of regular bins. */
               Axis::bin_t left,          /*!< The lower edge of the lowest bin.  */
               Axis::bin_t right,         /*!< The upper edge of the highest bin. */
               const std::string& xtitle, /*!< The title of the x axis. */
               const std::string& path="" /*!< Path if in directories within root file */);

  //! Deallocate memory.
  ~Histogram1D();

  //! Add another histogram.
  void Add(const Histogram1Dp other, data_t scale);

  //! Increment a histogram bin.
  void Fill(Axis::bin_t x,  /*!< The x axis value. */
            data_t weight=1 /*!< How much to add to the corresponding bin content. */)
  {
#ifdef H1D_USE_BUFFER
    buffer.push_back(buf_t(x, weight)); if( buffer.size()>=buffer_max ) FlushBuffer();
#else
    FillDirect(x, weight);
#endif /* H1D_USE_BUFFER */
  }

  //! Get the contents of a bin.
  /*! \return The bin content.
   */
  data_t GetBinContent(Axis::index_t bin /*!< The bin to look at. */);

  //! Get the x axis of the histogram.
  /*! \return The histogram's x axis.
   */
  [[nodiscard]] const Axis& GetAxisX() const
  { return xaxis; }

  //! Get the number of entries in the histogram.
  /*! \return The histogram's entry count.
   */
  [[nodiscard]] int GetEntries() const
  { return entries; }

  //! Clear all bins of the histogram.
  void Reset();

  //! Directly increment the histogram. Inlined for optimal performance.
  inline void FillDirect(const buf_t &element)
  {
      entries += 1;
      data[xaxis.FindBin( element.x )] += element.w;
  }

private:
  //! Increment a histogram bin directly, bypassing the buffer.
  void FillDirect(Axis::bin_t x,  /*!< The x axis value. */
                  data_t weight=1 /*!< How much to add to the corresponding bin content. */);

#ifdef H1D_USE_BUFFER
  //! Flush the data buffer.
  void FlushBuffer();
#endif /* H1D_USE_BUFFER */

  //! The x axis of the histogram;
  const Axis xaxis;

  //! The number of entries in the histogram.
  int entries;

  //! The bin contents, including the overflow bins.
  data_t *data;

#ifdef H1D_USE_BUFFER
  buffer_t buffer;
  static const unsigned int buffer_max = 1024;
#endif /* H1D_USE_BUFFER */
};

#endif /* HISTOGRAM1D_H_ */