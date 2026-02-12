#include "utils.h"

namespace download_manager::utils {
  std::vector<cpr::Range> split_ranges(size_t total, size_t parts) {
	std::vector<cpr::Range> ranges;
	if (parts == 0 || total == 0)
		return ranges;

	parts = std::min(parts, total);

	for (size_t i = 0; i < parts; ++i) {
		size_t begin = (i * total) / parts;
		size_t end   = ((i + 1) * total) / parts - 1;

		ranges.emplace_back(begin, end);
	}

	return ranges;
  }
}
