#ifndef CDOWNLOAD_MANAGER_UTILS_H
#define CDOWNLOAD_MANAGER_UTILS_H

#include <cstddef>
#include <string>  // IWYU pragma: keep
#include <vector>  // IWYU pragma: keep
#include <cpr/range.h>

namespace download_manager::utils {
  std::vector<cpr::Range> split_ranges(size_t total, size_t parts);
}

#endif // CDOWNLOAD_MANAGER_UTILS_H
