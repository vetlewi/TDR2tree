//
// Created by Vetle Wegner Ingeberg on 18/02/2021.
//

#ifndef TDRPARSER_H
#define TDRPARSER_H

#include <TDRChannel.h>
#include <TDREntry.h>

#ifndef NUM_CRATES
  #define NUM_CRATES 2
#endif  // NUM_CRATES

#ifndef NUM_SLOTS
  #define NUM_SLOTS 8
#endif  // NUM_SLOTS

#ifndef NUM_CHANNELS
  #define NUM_CHANNELS 16
#endif  // NUM_CHANNELS

#include <thread>

namespace TDR
{

    /*!
     * Find the first header in a segment of memory
     */
    const char *FindHeader(const char *begin, const char *end);

    /*!
     * Find the next header in a segment of memory
     */
    const char *FindNextHeader(const char *begin, const char *end);

  /*!
   * Find the beginning of each TDR buffer in a file.
   * \param begin is the pointer to the beginning of a file
   * \param end is the pointer to the end of the file
   * \return A vector with pointers to the start of each TDR buffer
   */
  std::vector<const char *> FindHeaders(const char *begin, const char *end);

  /*!
   * Find all entries in a single file.
   * \param begin pointer to start of the file
   * \param end pointer to the end of the file
   * \param threads number of threads to use (1 = single thread)
   * \return Vector with all entries in the file
   */
  std::vector<Entry_t> ParseFile(const char *begin, const char *end,
                                 const size_t &threads = std::thread::hardware_concurrency());

  class Parser
  {

  private:

    //! Array with Channel_t object for each possible address.
    Channel_t channels[NUM_CRATES * NUM_SLOTS * NUM_CHANNELS];

    /*!
     * Set info field for all channels within a slot
     * \param info field
     */
    void SetInfo(const TDR_info_type_t *info);

    /*!
     * Initialize info fields
     * \param buf pointer to buffer to set info fields from
     */
    void Initialize_info(const char *buf);

    /*!
     * Set an event for a channel
     * \param event
     */
    void SetEntry(const TDR_event_type_t *event);

    /*!
     * Parse a buffer. Will populate the channels array
     * \param buf pointer to buffer header
     */
    void Parse_buffer(const char *buf);

    /*!
     * Loops over all channels and collects finished entries.
     * \param buf pointer to a buffer
     * \return vector with all recreated entries (ie. actual events)
     */
    std::vector<Entry_t> Collect(const char *buf);

  public:

    explicit Parser(const char *buffer = nullptr){ Initialize_info(buffer); }

    /*!
     * Parse a buffer.
     * \param buf pointer to the buffer header
     * \param eof indicate if end of file has been reached (collect everything now)
     * \return A vector of entries (events).
     */
    std::vector<Entry_t> ParseBuffer(const char *buf, const bool &eof);


  };


}

#endif  // TDRPARSER_H
