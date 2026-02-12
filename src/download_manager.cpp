#include "download_manager.h"
#include "config.h"
#include "ui.h"
#include <argparse/argparse.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

DownloadManager::DownloadManager() = default;

int DownloadManager::run(int argc, char *argv[]) {
    argparse::ArgumentParser program("app");

    program.add_argument("--url")
        .help("url do download")
        .default_value(std::string(""));
    program.add_argument("--header").flag();
    program.add_argument("--output")
        .help("pasta de destino do download")
        .default_value(std::string("."));

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

    if (header_only && !url.empty()) {
        PreDownloadInfo::check_info(url, true);
        return EXIT_SUCCESS;
    }

    auto output_dir = program.get<std::string>("--output");

    if (!fs::is_directory(output_dir)) {
        std::cerr << "Caminho invalido: " << output_dir << " nao e uma pasta valida." << std::endl;
        std::cerr << "Usando pasta de execucao atual." << std::endl;
        output_dir = ".";
    }

    const AppConfig config = AppConfig::load();
    AppUI ui(config);
    ui.run(url, output_dir);

    return EXIT_SUCCESS;
}

bool DownloadManager::should_split(const size_t size, const bool accept_ranges) {
    constexpr size_t MIN_SPLIT_SIZE = 5 * 1024 * 1024; // 5MB
    return accept_ranges && size >= MIN_SPLIT_SIZE;
}

DownloadManager::~DownloadManager() = default;
