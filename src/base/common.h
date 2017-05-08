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
using offset_t = u32;

#if !defined(__BYTE_ORDER__) || (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ && __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__)
#error __BYTE_ORDER__ undefined
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
constexpr bool IS_LITTLE_ENDIAN = true;
#define IS_LITTLE_ENDIAN_P (true)
#else
constexpr bool IS_LITTLE_ENDIAN = false;
#define IS_LITTLE_ENDIAN_P (false)
#endif


namespace hidden
{
  struct u16
  {
    u16(::u16 v) : data(v) { }
    operator ::u16() const { return data; }
  private:
    ::u16 data;
  };
  struct u16re
  {
    u16re(::u16 v) : data(v) { }
    operator ::u16() const
    {
      ::u16 v = ((data & 0xFF) << 8) | ((data & 0xFF00) >> 8);
      return v;
    }
  private:
    ::u16 data;
  };
}

using u16le = std::conditional<IS_LITTLE_ENDIAN, hidden::u16, hidden::u16re>::type;
using u16be = std::conditional<IS_LITTLE_ENDIAN, hidden::u16re, hidden::u16>::type;


constexpr size_t KB16 = 16384;

enum class ZlibResult : int;

namespace utils
{
  std::string humanReadableSize(size_t bytes, bool si);
  
  int inflate(byte* src, size_t length, byte* dest, size_t destLength);
}
