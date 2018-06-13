//
//  main.cpp
//  CSOTool
//
//  Created by Jack on 12/03/17.
//  Copyright © 2017 Jack. All rights reserved.
//

#include "repository.h"
#include "tbx/hash/hash.h"
#include "tbx/base/common.h"

#include <iostream>

class common
{
public:
  static file_handle openFile(const path& path)
  {
    if (!path.exists())
      throw exceptions::file_not_found(path);
    
    file_handle handle = file_handle(path, file_mode::READING);
    
    if (!handle)
      throw exceptions::error_opening_file(path);
    
    return handle;
  }
  
  static inline void executeChunkedOperationOnFile(const file_handle& in, size_t bufferSize, const std::function<bool(byte* data, size_t length)>)
  {
    size_t length = in.length();
    size_t position = 0;
    
    byte* buffer = new byte[bufferSize];
    
    if (!buffer)
      throw exceptions::not_enough_memory("executeChunkOperationOnFile: not enough memory to allocate buffer");
    
    /*while (position < length)
    {
      size_t actual =
    }*/
    
    
    
    
    delete [] buffer;
  }
};


static void crc32command(const repository::CommandEnvironment& env)
{
  args::ArgumentParser parser = env.command.buildParser();
  
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag uppercase(parser, "uppercase", "Print result in uppercase format", {'u', "uppercase"}, false);
  args::Positional<std::string> argpath(parser, "path", "File to calculate CRC32 hash on");
  
  //parser.Help(std::cout);
  
  parser.ParseArgs(env.begin, env.end);
  
  path path = args::get(argpath);
  
  hash::crc32_digester digester;
  
  hash::crc32_t value = digester.compute(path);
  
  std::cout << std::hex << (uppercase ? std::uppercase : std::nouppercase) << value << std::endl;
}

static void md5command(const repository::CommandEnvironment& env)
{
  args::ArgumentParser parser = env.command.buildParser();
  
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
  args::Flag uppercase(parser, "uppercase", "Print result in uppercase format", {'u', "uppercase"}, false);
  args::Positional<std::string> argpath(parser, "path", "File to calculate MD5 hash on");
  
  //parser.Help(std::cout);
  
  parser.ParseArgs(env.begin, env.end);
  
  path path = args::get(argpath);
  
  hash::md5_digester digester;
  
  file_handle handle = common::openFile(path);
  
  //hash::md5_t value = digester.compute(path);
  
  //std::cout << (uppercase ? std::uppercase : std::nouppercase) << value << std::endl;
}

static const repository::CommandBuilder crc32Builder(repository::Command("crc32", "computes CRC32 hash of a file", crc32command));
