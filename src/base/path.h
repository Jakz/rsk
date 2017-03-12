#pragma once

#include <string>

class path
{
private:
  std::string data;
  
public:
  path(const char* data) : data(data) { }
  path(const std::string& data) : data(data) { }
  
  bool exists() const;
  
  const char* c_str() { return data.c_str(); }
  
  friend std::ostream& operator<<(std::ostream& os, const class path& path) { os << path.data; return os; }
};