#ifndef CDOWNLOAD_MANAGER_CONFIG_H
#define CDOWNLOAD_MANAGER_CONFIG_H

#include <string>

struct AppConfig {
    int max_connections = 8;
    int max_downloads = 3;
    int max_retries = 3;

    static AppConfig load();
    void save() const;
    static std::string config_path();
};

#endif // CDOWNLOAD_MANAGER_CONFIG_H
