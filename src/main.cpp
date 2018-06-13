//
//  main.cpp
//  CSOTool
//
//  Created by Jack on 12/03/17.
//  Copyright © 2017 Jack. All rights reserved.
//

#include "tbx/base/common.h"
#include "util/args.h"
#include "repository.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <cmath>
#include <cassert>

#include <zlib.h>

#define FATAL_ERROR(c, ...) if (c) { std::cout << __VA_ARGS__; return 1; }

#define PROG_NAME "rsk"

struct cso_header
{
  char magic[4]; // always CISO
  uint32_t header_size; // variable value
  uint64_t uncompressed_size; // uncompressed total size
  uint32_t block_size; // size of block (usually 2048)
  uint8_t version; // version
  uint8_t index_shift; // left shift of the offset in the offset list
  uint8_t unused[2];
} __attribute((packed));

size_t file_length(FILE* file)
{
  size_t current = ftell(file);
  fseek(file, 0, SEEK_END);
  size_t length = ftell(file);
  fseek(file, current, SEEK_SET);
  return length;
}

void test();

void listCommand(const repository::CommandEnvironment& env)
{
  using namespace std;
  using namespace repository;
  const Repository* repo = Repository::instance();
  
  constexpr size_t indent = 4;
  vector<Command> commands(repo->begin(), repo->end());
  sort(commands.begin(), commands.end(), [](const Command& c1, const Command& c2) {
    return c1.name < c2.name;
  });
   
  cout << "Available commands:" << endl;
  for (const Command& command : commands)
  {
    cout << string(indent, ' ') << command.name << ": " << command.description << endl;
  }
}

namespace formats { namespace cue {
  struct Cue { };
  struct Environment { };
  struct istring
  {
    std::string data;
    
    bool operator==(const std::string& other) const { return strings::caseInsensitiveEqual(data, other); }
    
    istring(const std::string& data) : data(data) { }
  };
  
  Cue parse(Environment& env, const std::vector<istring>& tokens);
  std::vector<istring> tokenize(const std::string& line);
} }

#include "files/duplicate_finder.h"
#include "tbx/base/file_system.h"

int main(int argc, const char * argv[])
{
  auto* repository = repository::Repository::instance();
  
  //const std::vector<std::string> args(argv + 1, argv + argc);
  const std::vector<std::string> args = /*{"list"};*/{"crc32", "/Users/jack/Desktop/hiroshi3.rar"};
  args::ArgumentParser parser(PROG_NAME);
  parser.helpParams.showProglineOptions = false;
  parser.helpParams.showTerminator = false;
  parser.Prog(PROG_NAME);
  parser.ProglinePostfix("{arguments}");
  repository->registerCommand(repository::Command("list", "print all available commands", listCommand));
  auto map = repository->prepareCommandMap();
  args::MapPositional<std::string, std::reference_wrapper<const repository::Command>> command(parser, "command", "Command to execute", map, map.find("list")->second);
  command.KickOut(true);

  try
  {
    auto next = parser.ParseArgs(args);
    const auto& commandReference = args::get(command);
    commandReference.get().callback(repository::CommandEnvironment(repository::Repository::instance(), commandReference.get(), PROG_NAME, next, std::end(args)));
  }
  catch (args::Help e)
  {
    std::cout << parser;
    return 0;
  }
  catch (args::ValidationError e)
  {
    std::cout << e.parser;
    return 0;
  }
  catch (args::MapError e)
  {
    std::cout << "Unknown or unspecified command, use '" PROG_NAME " list' to see available commands." << std::endl;
    return 1;
  }
  catch (args::Error e)
  {
    std::cerr << e.what() << std::endl;
    std::cerr << parser;
    return 1;
  }
  catch (exceptions::file_not_found e)
  {
    std::cerr << "Error, file not found: " << e.what() << std::endl;
  }
  
  return 0;
  
  path path = "/Volumes/Vicky/Roms/roms/psp/SORTED/Action/Diner Dash [EUR].cso";
  
  if (!path.exists())
    throw exceptions::file_not_found(path);
  
  FILE* in = fopen(path.c_str(), "rb");
  
  FATAL_ERROR(!in, "Error opening input file");

  size_t length = file_length(in);
  
  FATAL_ERROR(length < sizeof(cso_header), "Invalid file format");
  
  cso_header header;
  fread(&header, sizeof(header), 1, in);
  FATAL_ERROR(memcmp(header.magic, "CISO", 4) != 0, "Invalid file header");

  size_t offsetCount = ceil(((float)header.uncompressed_size) / header.block_size) + 1;
  
  FATAL_ERROR(length < sizeof(uint32_t)*offsetCount + sizeof(cso_header), "Invalid file format");
  
  std::vector<uint32_t> offsets;
  offsets.resize(offsetCount);
  
  fread(&offsets[0], sizeof(uint32_t), offsetCount, in);
  
  FATAL_ERROR(offsets.back() != length, "Invalid last offset value");
  
  {
    size_t uncompressedBlockCount = std::count_if(offsets.begin(), offsets.end(), [](const uint32_t& offset) { return offset & 0x80000000; });
    
    std::cout << "====================" << std::endl;
    std::cout << " FILE INFO" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << "Filepath: " << path << std::endl;
    std::cout << "Compressed size: " << strings::humanReadableSize(length, true) << std::endl;
    std::cout << "Uncompressed size: " << strings::humanReadableSize(header.uncompressed_size, true) << std::endl;
    std::cout << "Compression ratio: " << std::setprecision(3) << header.uncompressed_size / (float)length << "x" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << "Block size: " << header.block_size << std::endl;
    std::cout << "Block count: " << offsetCount << std::endl;
    std::cout << "Uncompressed blocks count: " << uncompressedBlockCount << std::endl;
  }
  
  byte* buffer = new byte[header.block_size];
  
  FILE* out = fopen("/Users/jack/Desktop/bomberman.iso", "wb");
  
  for (size_t i = 0; i < offsetCount - 1; ++i)
  {
    uint32_t offset = offsets[i];
    uint32_t realOffset = (offset & ~0x80000000) << header.index_shift;
    uint32_t blockCompressedSize = (offsets[i+1] & ~0x80000000) - realOffset;
    
    // compressed
    if (offset & 0x80000000)
    {
      assert(blockCompressedSize == header.block_size);
      fread(buffer, sizeof(byte), header.block_size, in);
    }
    else
    {
      byte* block = new byte[blockCompressedSize];
      
      fseek(in, realOffset, SEEK_SET);
      fread(block, sizeof(byte), blockCompressedSize, in);
      
      int r = utils::inflate(block, blockCompressedSize, buffer, header.block_size);
      
      assert(!r);
      
      delete [] block;
    }
    
    fwrite(buffer, sizeof(byte), header.block_size, out);
  }
  
  size_t currentPosition = ftell(out);
  
  byte* padding = new byte[16384];
  std::fill(padding, padding+16384, 0);
  
  while (currentPosition < 137265152)
  {
    size_t toWrite = std::min(137265152ULL - currentPosition, 16384ULL);
    fwrite(padding, sizeof(byte), toWrite, out);
    currentPosition = ftell(out);
  }

  delete [] padding;
  
  fclose(out);
  fclose(in);
  
  delete [] buffer;
}

static void run(const repository::CommandEnvironment&);
static const repository::CommandBuilder builder(repository::Command("cso", "CSO format conversions", run));

static void run(const repository::CommandEnvironment& env)
{
  args::ArgumentParser parser = builder.command().buildParser();
  args::Group group(parser, "operating mode", args::Group::Validators::Xor);
  args::Flag extract(parser, "extract", "extract ISO from CSO", {"e", "extract"});
  args::Flag create(parser, "create", "create CSO from ISO", {"c", "create"});
  args::Flag info(parser, "info", "get CSO file information", {"i", "info"});

  args::Positional<std::string> src(parser, "src", "source path");
  args::Positional<std::string> dest(parser, "dest", "destination path");
  
  parser.ParseArgs(env.begin, env.end);
}
