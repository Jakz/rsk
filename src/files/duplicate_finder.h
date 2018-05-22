#pragma once

#include "base/common.h"
#include "base/path.h"

#include <unordered_map>

class DuplicateFinder
{
private:
  struct SearchPath
  {
    const path path;
    const bool recursive;
    const path::predicate exclusionPredicate;
  };
  
  std::vector<SearchPath> searchPaths;
  
public:
  struct result_data
  {
    relative_path path;
    u32_optional size;
    u32_optional crc;
    
    result_data(relative_path path) : path(path) { }
  };
  
  
  struct result_set
  {
    using data_type = std::unordered_multimap<path, result_data, path::hash>;
  public:
    data_type files;
  public:
    using iterator = decltype(files)::const_iterator;
    iterator begin() const { return files.begin(); }
    iterator end() const { return files.end(); }
    size_t size() const { return files.size(); }
  };
  
  
  
  void addSearchPath(const path path, bool recursive, path::predicate exclusionPredicate = [](const class path&) { return false; });
  void clearSearchPaths() { searchPaths.clear(); }
  
  result_set scanPaths();
  
};
