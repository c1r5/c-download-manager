#include "downloader.h"
#include "structs.h"
#include "utils.h"
#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <future>
#include <iostream>
#include <string>

using namespace download_manager::utils;

PreDownloadInfo PreDownloadInfo::check_info(const std::string &url, const bool &header_only) {
  PreDownloadInfo info{ false, 0, url, ""};

	try {
		const auto response = cpr::Head(cpr::Url{url}, cpr::Header{{"Accept-Encoding", "identity"}});
		if (header_only) {
      std::cout << response.raw_header << std::endl;
      return info;
    }
		if (const auto it = response.header.find("Accept-Ranges"); it != response.header.end()) {
			info.accept_ranges = (it->second == "bytes");
		}

		if (const auto it = response.header.find("Content-Length"); it != response.header.end()) {
			try {
				info.content_size = static_cast<size_t>(std::stoull(it->second));
			} catch (...) {}
		}

		if (const auto it = response.header.find("Content-Disposition"); it != response.header.end()) {
			info.filename = extract_filename_from_header(it->second);
		}

		if (info.filename.empty()) {
			info.filename = extract_filename_from_url(url);
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}

	return info;
}

void SingleDownloader::download(const DownloadOptions &options) {
	auto start = std::chrono::steady_clock::now();
	emit({STARTED, 0, options.c_size, 0.0, 0});

	std::ofstream ofs(options.out, std::ios::binary);

	if (!ofs.is_open()) {
		std::cerr << "Failed to open file for writing: " << options.out << std::endl;
		emit({FAILED, 0, options.c_size, 0.0});
		return;
	}

	if (const cpr::Response response = cpr::Download(ofs, cpr::Url{options.url }); response.status_code == 200) {
		double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
		emit({FINISHED, options.c_size, options.c_size, elapsed, 0});
		emit({FINISHED, options.c_size, options.c_size, elapsed});
	} else {
		double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
		emit({FAILED, 0, options.c_size, elapsed, 0});
		emit({FAILED, 0, options.c_size, elapsed});
	}
}

void ParalellDownloader::download(const DownloadOptions &options) {
  auto start = std::chrono::steady_clock::now();
  emit({STARTED, 0, options.c_size, 0.0});

  const auto ranges =
      split_ranges(options.c_size, static_cast<size_t>(thread_count));

  // Emit initial state for each thread so the UI knows about them
  for (int i = 0; i < static_cast<int>(ranges.size()); ++i) {
    size_t chunk_size = ranges[i].finish_at - ranges[i].resume_from + 1;
    emit({STARTED, 0, chunk_size, 0.0, i});
  }

  std::vector<std::future<void>> futures;
  futures.reserve(ranges.size());

  for (int i = 0; i < static_cast<int>(ranges.size()); ++i) {
    const auto& r = ranges[i];
    futures.emplace_back(std::async(std::launch::async, [this, &options, r, start, i] {
      auto thread_start = std::chrono::steady_clock::now();
      size_t chunk_size = r.finish_at - r.resume_from + 1;

      std::string range_value = "bytes=" + std::to_string(r.resume_from) + "-" +
                                std::to_string(r.finish_at);

      const auto response =
          cpr::Get(cpr::Url{options.url}, cpr::Header{{"Range", range_value}});

      double thread_elapsed = std::chrono::duration<double>(
          std::chrono::steady_clock::now() - thread_start).count();

      if (response.status_code == 206) {
        std::ofstream file(options.out,
                           std::ios::binary | std::ios::in | std::ios::out);
        file.seekp(r.resume_from);
        file.write(response.text.data(),
                   static_cast<std::streamsize>(response.text.size()));
        file.close();

        emit({FINISHED, response.text.size(), chunk_size, thread_elapsed, i});
      } else {
        emit({FAILED, 0, chunk_size, thread_elapsed, i});
      }
    }));
  }

	for (auto& f : futures) {
		f.get();
	}

	double total_elapsed = std::chrono::duration<double>(
	    std::chrono::steady_clock::now() - start).count();
	emit({FINISHED, options.c_size, options.c_size, total_elapsed});
}

