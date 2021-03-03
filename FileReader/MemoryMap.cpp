#include "MemoryMap.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>

using namespace IO;

MemoryMap::MemoryMap(const char *fname) : memory_buffer(nullptr), file(0), size(0) {
  struct stat sb;

  file = open(fname, O_RDONLY, NULL);

  if (file == -1) {
    std::string errmsg = "Unable to open file '";
    errmsg += fname;
    errmsg += "', got error '";
    errmsg += strerror(errno);
    errmsg += "'.";
    throw std::runtime_error(errmsg);
  }

  // Check size of file
  if (fstat(file, &sb) == -1) {
    std::string errmsg = "Unable to estimate file size, got error '";
    errmsg += strerror(errno);
    errmsg += "'.";
    throw std::runtime_error(errmsg);
  }

  size = sb.st_size;

  // Try to memory map the file
  memory_buffer = reinterpret_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, file, 0));
  if (memory_buffer == MAP_FAILED) {
    std::string errmsg = "Unable to memory map file, got error '";
    errmsg += strerror(errno);
    errmsg += "'.";
    throw std::runtime_error(errmsg);
  }
}

MemoryMap::~MemoryMap() {
  // First we need de-map the memory.
  if (memory_buffer) {
    if (munmap(reinterpret_cast<void *>(memory_buffer), size) == -1) {
      std::string errmsg = "Unable to de-allocate mapped memory, got error '";
      errmsg += strerror(errno);
      errmsg += "'.";
      std::cerr << errmsg << std::endl;
    }
  }
  close(file);
}
