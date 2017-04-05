#include <iostream>
#include "base/common.h"
#include "base/exceptions.h"

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

#include "repository.h"

static void run(const std::string& name, repository::arg_iterator begin, repository::arg_iterator end);
static const repository::CommandBuilder builder(repository::Command("crc32", "computes CRC32 hash of a file", run));

static void run(const std::string& name, repository::arg_iterator begin, repository::arg_iterator end)
{
  args::ArgumentParser parser = builder.command().buildParser();
  
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag uppercase(parser, "uppercase", "Print result in uppercase format", {'u', "uppercase"}, false);
  args::Positional<std::string> argpath(parser, "path", "File to calculate MD5 hash on");
  
  //parser.Help(std::cout);
  
  parser.ParseArgs(begin, end);
  
  path path = args::get(argpath);
  
  if (!path.exists())
    throw exceptions::file_not_found(path);
  
  file_handle handle = file_handle(path, file_mode::READING);
  
  if (!handle)
    throw exceptions::error_opening_file(path);

  size_t fileLength = handle.length();
  size_t current = 0;
  std::unique_ptr<byte[]> bufferPtr = std::unique_ptr<byte[]>(new byte[KB16]);
  byte* buffer = bufferPtr.get();
  
  if (!init)
    precomputeLUT();
  
  u32 crc = 0;
  
  while (current < fileLength)
  {
    size_t amountToProcess = std::min(KB16, fileLength - current);
    
    if (handle.read(buffer, 1, amountToProcess))
      crc = updateCRC(buffer, amountToProcess, crc);
    else
      throw exceptions::error_reading_from_file(path);
    
    current += KB16;
  }
  
  std::cout << std::hex << crc << std::endl;
}
