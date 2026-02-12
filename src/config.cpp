#include "config.h"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

std::string AppConfig::config_path() {
    const char* home = std::getenv("HOME");
    fs::path config_dir = fs::path(".");
    fs::create_directories(config_dir);
    return (config_dir / "config.ini").string();
}

AppConfig AppConfig::load() {
    AppConfig config;
    std::string path = config_path();

    std::ifstream file(path);
    if (!file.is_open()) {
        config.save();
        return config;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '[') continue;

        auto eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        try {
            if (key == "max_connections") config.max_connections = std::stoi(value);
            else if (key == "max_downloads") config.max_downloads = std::stoi(value);
            else if (key == "max_retries") config.max_retries = std::stoi(value);
        } catch (const std::exception& e) {
            std::cerr << "Erro ao ler config '" << key << "': " << e.what() << std::endl;
        }
    }

    return config;
}

void AppConfig::save() const {
    std::ofstream file(config_path());
    if (!file.is_open()) return;

    file << "[download]" << std::endl;
    file << "max_connections=" << max_connections << std::endl;
    file << "max_downloads=" << max_downloads << std::endl;
    file << "max_retries=" << max_retries << std::endl;
}
