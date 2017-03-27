#pragma once

#include <exception>
#include <string>

#include "path.h"

namespace exceptions
{
  class rsk_exception : public std::exception
  {
    
  };
  
  class file_not_found : public rsk_exception
  {
  private:
    path path;
    
  public:
    file_not_found(const class path& path) : path(path) { }

    const char* what() const noexcept override { return path.c_str(); }

  };
  
  class error_opening_file : public rsk_exception
  {
  private:
    path path;
    
  public:
    error_opening_file(const class path& path) : path(path) { }
    
    const char* what() const noexcept override { return path.c_str(); }
  };
  
  class error_reading_from_file : public rsk_exception
  {
  private:
    path path;
    
  public:
    error_reading_from_file(const class path& path) : path(path) { }
    
    const char* what() const noexcept override { return path.c_str(); }
  };
}
