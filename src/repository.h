#pragma once

#include "base/common.h"

namespace repository
{
  using arg_list = std::vector<std::string>;
  using arg_iterator = std::vector<std::string>::const_iterator;
  using arg_command = std::function<void(const std::string&, arg_iterator, arg_iterator)>;
  
  class command
  {
    std::string name;
    std::string description;
    arg_command callback;
  };
  
  class Repo
  {
  private:
    
  public:
  };
}