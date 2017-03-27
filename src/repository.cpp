#include "repository.h"

#include <unordered_map>

using namespace repository;

Repository* Repository::instance()
{
  static Repository repository;
  return &repository;
}

std::unordered_map<std::string, arg_command> Repository::prepareCommandMap()
{
  std::unordered_map<std::string, repository::arg_command> commandMap;
  
  std::transform(commands.begin(), commands.end(), std::inserter(commandMap, commandMap.end()), [](const Command& command) {
    return std::make_pair(command.name, command.callback);
  });
  
  return commandMap;
}

CommandBuilder::CommandBuilder(const Command& command)
{
  Repository::instance()->registerCommand(command);
}
