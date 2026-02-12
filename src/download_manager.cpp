#include "download_manager.h"
#include "constants.h"
#include "downloader.h"
#include "structs.h"
#include <argparse/argparse.hpp>
#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include "ui.h"

DownloadManager::DownloadManager() = default;

int DownloadManager::run(int argc, char *argv[]) {
  argparse::ArgumentParser program("app");

	program.add_argument("--url").help("url");
  program.add_argument("--header").flag();
  program.add_argument("--output").help("output file");
	
  try {
		program.parse_args(argc, argv);
	} catch (const std::exception &err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::cout << program.help().str() << std::endl;
		return 1;
	}

	const auto url = program.get<std::string>("--url");
	const auto header_only = program.get<bool>("--header");

	const PreDownloadInfo info = PreDownloadInfo::check_info(url, header_only);

	if (header_only) return EXIT_SUCCESS;

	const auto output_file = program.get<std::string>("--output");

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
  DownloadManagerUI ui {};

  downloader->add_observer(&ui);
	downloader->download(options);

	return EXIT_SUCCESS;
}

bool DownloadManager::should_split(const size_t size, const bool accept_ranges) {
	constexpr size_t MIN_SPLIT_SIZE = 5 * 1024 * 1024; // 5MB
	return accept_ranges && size >= MIN_SPLIT_SIZE;
}

DownloadManager::~DownloadManager() = default;
