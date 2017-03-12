#pragma once

#include <exception>
#include <string>

#include "path.h"

namespace excpts
{
  class rsk_exception : public std::exception
  {
    
  };
  
  class file_not_found_exception : public rsk_exception
  {
  private:
    path path;
    std::string message;
    
  public:
    file_not_found_exception(const class path& path, const std::string& message) :
    path(path), message(message) { }
  };
}