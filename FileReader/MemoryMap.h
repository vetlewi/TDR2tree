#pragma once

#include <cstddef>

namespace IO {

  /*!
   * \brief Memory-mapped input file that can be accessed by multiple threads
   */
  class MemoryMap {
  private:
    /*! The memory mapped file */
    char *memory_buffer;

    /*! File descriptor */
    int file;

    /*! Size of file in bytes */
    size_t size;

  public:
    explicit MemoryMap(const char *fname);

    MemoryMap(MemoryMap &) = delete;
    MemoryMap(MemoryMap &&) = delete;

    ~MemoryMap();

    template <typename T = char> const T *GetPtr() const {
      return reinterpret_cast<const T *>(memory_buffer);
    }

    template <typename T = char> size_t GetSize() const { return size / sizeof(T); }
  };

}  // namespace IO
