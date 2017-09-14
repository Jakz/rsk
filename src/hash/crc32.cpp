#include <iostream>

using u32 = uint32_t;
using byte = uint8_t;

constexpr size_t TABLE_SIZE = 256;
constexpr u32 POLYNOMIAL = 0xEDB88320;

bool init = false;
u32 lut[TABLE_SIZE] = {0};

void precomputeLUT()
{
  for (u32 i = 0; i < 256; ++i)
  {
    u32 crc = i;
    for (u32 j = 0; j < 8; j++)
      crc = (crc >> 1) ^ (-int(crc & 1) & POLYNOMIAL);
    
    lut[i] = crc;
  }
  
  init = true;
}

u32 updateCRC(const byte* data, size_t length, u32 previous = 0)
{
  u32 crc = ~previous;
  
  while (length--)
    crc = (crc >> 8) ^ lut[(crc & 0xFF) ^ *data++];
  
  return ~crc;
}
