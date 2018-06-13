#pragma once

#include "tbx/base/common.h"
#include "util/args.h"
#include <unordered_map>

namespace repository
{
  using arg_list = std::vector<std::string>;
  using arg_iterator = std::vector<std::string>::const_iterator;
  
  class Repository;
  struct Command;
  
  class CommandEnvironment
  {
  public:
    Repository* repository;
    const Command& command;
    const std::string& name;
    arg_iterator begin;
    arg_iterator end;
    
  public:
    CommandEnvironment(Repository* repository, const Command& command, const std::string& name, arg_iterator begin, arg_iterator end) :
      repository(repository), command(command), name(name), begin(begin), end(end)
    { }
  };
  
  using arg_command = std::function<void(const CommandEnvironment&)>;
  
  struct Command
  {
    std::string name;
    std::string description;
    arg_command callback;

    Command(const std::string& name, const std::string& description, const arg_command& callback) :
    name(name), description(description), callback(callback) { }
    
    args::ArgumentParser buildParser() const;
  };
  

  
  class Repository
  {
  using map_t = std::unordered_map<std::string, std::reference_wrapper<const Command>>;
    
  private:
    std::vector<Command> commands;
    
  public:
    void registerCommand(const Command& command) { commands.push_back(command); }
    map_t prepareCommandMap();
    
    using iterator = std::vector<Command>::const_iterator;

    iterator begin() const { return commands.begin(); }
    iterator end() const { return commands.end(); }
    
    static Repository* instance();
  };
  
  class CommandBuilder
  {
  private:
    const Command _command;
  public:
    CommandBuilder(const Command& command);
    const Command& command() const { return _command; }
  };
}
