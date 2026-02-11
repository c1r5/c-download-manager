#include "download_manager.hpp"
#include "cpr/api.h"
#include "cpr/cprtypes.h"
#include <argparse/argparse.hpp>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <future>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#include "constants.hpp"

DownloadManager::DownloadManager() = default;

int DownloadManager::run(int argc, char *argv[]) {
	argparse::ArgumentParser program("dlman");

	program.add_argument("--url").help("download url").required();
	program.add_argument("--output").help("output file").required();

	try {
		program.parse_args(argc, argv);
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::cout << program.help().str() << std::endl;
		return 1;
	}

	const auto url = program.get<std::string>("--url");
	const auto output_file = program.get<std::string>("--output");

	const PreDownloadInfo info = PreDownloadInfo::check_info(url);

	std::ofstream file(output_file, std::ios::binary);
	file.seekp(static_cast<std::streamoff>(info.content_size) - 1);
	file.write("", 1);
	file.close();

	std::unique_ptr<DefaultDownloader> downloader;
	if (should_split(info.content_size, info.accept_ranges)) {
		downloader = std::make_unique<ParalellDownloader>(constants::MAX_CONNECTIONS);
	} else {
		downloader = std::make_unique<SingleDownloader>();
	}

	DownloadOptions options {info.url, output_file, info.content_size};

	downloader->download(options);

	return EXIT_SUCCESS;
}

bool DownloadManager::should_split(const size_t size, const bool accept_ranges) {
	constexpr size_t MIN_SPLIT_SIZE = 5 * 1024 * 1024; // 5MB
	return accept_ranges && size >= MIN_SPLIT_SIZE;
}

std::vector<cpr::Range> DownloadManager::split_ranges(size_t total, size_t parts) {
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

DownloadManager::~DownloadManager() = default;

PreDownloadInfo PreDownloadInfo::check_info(const std::string &url) {
	bool accept_ranges = false;
	size_t content_size = 0;
	try {
		const auto response = cpr::Head(cpr::Url{url}, cpr::Header{{"Accept-Encoding", "identity"}});

		if (const auto it = response.header.find("Accept-Ranges"); it != response.header.end()) {
			accept_ranges = (it->second == "bytes");
		}

		if (const auto it = response.header.find("Content-Length"); it != response.header.end()) {
			try {
				content_size = static_cast<size_t>(std::stoull(it->second));
			} catch (const std::exception &) {
				content_size = 0;
			}
		}
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
	}

	PreDownloadInfo info{ accept_ranges, content_size, url};
	return info;
}

void SingleDownloader::download(const DownloadOptions &options) {
	std::ofstream ofs(options.out, std::ios::binary);

	if (!ofs.is_open()) {
		std::cerr << "Failed to open file for writing: " << options.out << std::endl;
		return;
	}

	if (const cpr::Response response = cpr::Download(ofs, cpr::Url{options.url }); response.status_code == 200) {
		std::cout << "File successfully downloaded and saved to " << options.out << std::endl;
	} else {
		std::cerr << "Failed to download file. Status code: " << response.status_code << std::endl;
	}
}

void ParalellDownloader::download(const DownloadOptions &options) {
	const auto ranges = DownloadManager::split_ranges(options.c_size,  static_cast<size_t>(thread_count));

	std::vector<std::future<void>> futures;
	futures.reserve(ranges.size());

	for (const auto& r : ranges) {
		futures.emplace_back(
			std::async(std::launch::async,
				[&options, r] {
					const auto thread_id = std::this_thread::get_id();

					std::string range_value = "bytes=" + std::to_string(r.resume_from) +
							  "-" + std::to_string(r.finish_at);

					const auto response = cpr::Get(
						cpr::Url{options.url},
						cpr::Header{{"Range", range_value}}
					);

					if (response.status_code == 206) { // 206 Partial Content
						std::ofstream file(options.out, std::ios::binary | std::ios::in | std::ios::out);
						file.seekp(r.resume_from);
						file.write(response.text.data(), static_cast<std::streamsize>(response.text.size()));
						file.close();

						std::cout << "Thread " << thread_id << "completed: "
								  << response.text.size() << " bytes\n";
					} else {
						std::cerr << "Thread " << thread_id << " failed with status: "
								  << response.status_code << "\n";
					}
				}
			)
		);
	}

	// Espera todas finalizarem
	for (auto& f : futures) {
		f.get(); // propaga exceções também
	}
}
