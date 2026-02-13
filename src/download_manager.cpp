#include "download_manager.h"
#include "config.h"
#include "ui.h"
#include <argparse/argparse.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace fs = std::filesystem;

DownloadManager::DownloadManager() = default;

int DownloadManager::run(int argc, char *argv[]) {
    auto logger = spdlog::rotating_logger_mt("app", "cdownload.log", 5 * 1024 * 1024, 2);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("cdownload-manager iniciado");

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
        spdlog::error("erro ao parsear argumentos: {}", err.what());
        std::cout << program.help().str() << std::endl;
        return 1;
    }

    const auto url = program.get<std::string>("--url");
    const auto header_only = program.get<bool>("--header");
    spdlog::info("args: url={}, header_only={}", url, header_only);

    if (header_only && !url.empty()) {
        PreDownloadInfo::check_info(url, true);
        return EXIT_SUCCESS;
    }

    auto output_dir = program.get<std::string>("--output");

    if (!fs::is_directory(output_dir)) {
        spdlog::warn("caminho invalido: {} nao e uma pasta valida, usando pasta atual", output_dir);
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
