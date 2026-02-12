#ifndef CDOWNLOAD_MANAGER_UTILS_H
#define CDOWNLOAD_MANAGER_UTILS_H

#include <cstddef>
#include <string>  // IWYU pragma: keep
#include <vector>  // IWYU pragma: keep
#include <cpr/range.h>

namespace download_manager::utils {
  std::vector<cpr::Range> split_ranges(size_t total, size_t parts);
  std::string extract_filename_from_url(const std::string& url);
  std::string extract_filename_from_header(const std::string& header_value);
}

#endif // CDOWNLOAD_MANAGER_UTILS_H
