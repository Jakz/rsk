#pragma once

#include "base/common.h"
#include "util/args.h"
#include <unordered_map>

namespace repository
{
  using arg_list = std::vector<std::string>;
  using arg_iterator = std::vector<std::string>::const_iterator;
  using arg_command = std::function<void(const std::string&, arg_iterator, arg_iterator)>;
  
  struct Command
  {
    std::string name;
    std::string description;
    arg_command callback;

    Command(const std::string& name, const std::string& description, const arg_command& callback) :
    name(name), description(description), callback(callback) { }
  };
  
  class Repository
  {
  private:
    std::vector<Command> commands;
    
  public:
    void registerCommand(const Command& command) { commands.push_back(command); }
    std::unordered_map<std::string, arg_command> prepareCommandMap();
    
    static Repository* instance();
  };
  
  class CommandBuilder
  {
  public:
    CommandBuilder(const Command& command);
  };
}
