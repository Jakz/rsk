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


args::ArgumentParser Command::buildParser() const
{
  args::ArgumentParser parser(this->description);
  parser.Prog("rsk " + this->name);
  parser.helpParams.flagindent = 3;
  parser.helpParams.showTerminator = false;
  return parser;
}


CommandBuilder::CommandBuilder(const Command& command) : _command(command)
{
  Repository::instance()->registerCommand(command);
}
