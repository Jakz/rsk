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

using s64 = int64_t;

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

struct u32_optional
{
private:
  u64 data;
  
public:
  u32_optional() : data(0xFFFFFFFFFFFFFFFFLL) { }
  u32_optional(u32 data) : data(data) { }
  
  bool isPresent() const { return (data & 0xFFFFFFFF) != 0; }
  void set(u32 data) { this->data = data; }
};

template <bool B, typename T, T trueval, T falseval>
struct conditional_value : std::conditional<B, std::integral_constant<T, trueval>, std::integral_constant<T, falseval>>::type { };
template<typename T> using predicate = std::function<bool(const T&)>;

template<typename T>
struct bit_mask
{
  using utype = typename std::underlying_type<T>::type;
  utype value;
  
  bool isSet(T flag) const { return value & static_cast<utype>(flag); }
  void set(T flag) { value |= static_cast<utype>(flag); }
  void reset(T flag) { value &= ~static_cast<utype>(flag); }
  void set(T flag, bool v) { if (v) set(flag); else reset(flag); }
  
  bit_mask<T>& operator&(T flag)
  {
    bit_mask<T> mask;
    mask.value = this->value & static_cast<utype>(flag);
    return mask;
  }
  
  bit_mask<T>& operator|(T flag)
  {
    bit_mask<T> mask;
    mask.value = this->value | static_cast<utype>(flag);
    return mask;
  }
  
  bool operator&&(T flag) const { return (value & static_cast<utype>(flag)) != 0; }
};


constexpr size_t KB16 = 16384;

enum class ZlibResult : int;

namespace utils
{
  std::string humanReadableSize(size_t bytes, bool si);
  
  int inflate(byte* src, size_t length, byte* dest, size_t destLength);
}

namespace strings
{
  std::string humanReadableSize(size_t bytes, bool si);
  bool isPrefixOf(const std::string& string, const std::string& prefix);
  
  std::string tolower(const std::string& other);
  inline bool caseInsensitiveEqual(const std::string& t1, const std::string& t2) { return tolower(t1) == tolower(t2); }
}
