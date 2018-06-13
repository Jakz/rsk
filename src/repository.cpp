#include "repository.h"

#include <unordered_map>

using namespace repository;

Repository* Repository::instance()
{
  static Repository repository;
  return &repository;
}

Repository::map_t Repository::prepareCommandMap()
{
  map_t commandMap;
  
  std::transform(commands.begin(), commands.end(), std::inserter(commandMap, commandMap.end()), [] (const Command& command) {
    return std::make_pair(command.name, std::ref(command));
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
