#pragma once

#include <cstdint>
#include <cmath>

#include <string>

#include "util/fmt/printf.h"
#include "exceptions.h"
#include "path.h"



using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using byte = u8;

namespace utils
{
  std::string humanReadableSize(size_t bytes, bool si);
}

namespace exceptions
{
  struct file_not_found : public std::exception
  {
  private:
    path path;
    
  public:
    file_not_found(const class path& path) : path(path) { }
    const char* what() const noexcept override { return path.c_str(); }
  };
}
