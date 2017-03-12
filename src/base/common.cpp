#include "common.h"

std::string utils::humanReadableSize(size_t bytes, bool si) {
  static constexpr char pre[][7] = { "kMGTPE", "KMGTPE"};
  
  
  int unit = si ? 1000 : 1024;
  if (bytes < unit) return std::to_string(bytes) + " B";
  int exp = std::log(bytes) / std::log(unit);
  
  return fmt::sprintf("%.1f %c%sB", bytes / std::pow(unit, exp), pre[si ? 1 : 0][exp-1], si ? "" : "i");
}