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

constexpr size_t KB16 = 16384;

enum class ZlibResult : int;

namespace utils
{
  std::string humanReadableSize(size_t bytes, bool si);
  
  int inflate(byte* src, size_t length, byte* dest, size_t destLength);
}
