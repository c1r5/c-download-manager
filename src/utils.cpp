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

  std::string extract_filename_from_url(const std::string& url) {
	std::string path = url;

	auto query_pos = path.find('?');
	if (query_pos != std::string::npos) path = path.substr(0, query_pos);

	auto frag_pos = path.find('#');
	if (frag_pos != std::string::npos) path = path.substr(0, frag_pos);

	auto last_slash = path.rfind('/');
	if (last_slash != std::string::npos && last_slash + 1 < path.size()) {
		return path.substr(last_slash + 1);
	}

	return "download";
  }

  std::string extract_filename_from_header(const std::string& header_value) {
	std::string filename;

	auto pos = header_value.find("filename=");
	if (pos == std::string::npos) return filename;

	pos += 9; // strlen("filename=")

	if (pos < header_value.size() && header_value[pos] == '"') {
		pos++;
		auto end = header_value.find('"', pos);
		if (end != std::string::npos) {
			filename = header_value.substr(pos, end - pos);
		}
	} else {
		auto end = header_value.find(';', pos);
		filename = header_value.substr(pos, end - pos);
	}

	return filename;
  }
}
