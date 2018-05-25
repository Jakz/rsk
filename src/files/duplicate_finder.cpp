#include "duplicate_finder.h"
#include "base/file_system.h"

void DuplicateFinder::addSearchPath(const path path, bool recursive, path::predicate exclusionPredicate)
{
  searchPaths.push_back({path, recursive, exclusionPredicate});
}

DuplicateFinder::result_set DuplicateFinder::scanPaths()
{
  result_set result;
  for (const SearchPath& searchPath : searchPaths)
  {
    auto files = FileSystem::i()->contentsOfFolder(searchPath.path, searchPath.recursive, searchPath.exclusionPredicate);
    
    std::transform(
      files.begin(),
      files.end(),
      std::inserter(result.files, result.files.begin()),
      [&searchPath](const path& path) {
        return std::make_pair(searchPath.path, result_data(relative_path(searchPath.path, path)));
      }
    );
  }
  
  return result;
}
