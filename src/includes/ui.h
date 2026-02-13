#ifndef CDOWNLOAD_MANAGER_UI_H
#define CDOWNLOAD_MANAGER_UI_H

#include "config.h"
#include "observer.h"
#include "structs.h"
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace ftxui {
class ScreenInteractive;
}

class DownloadObserverAdapter : public IObserver<DownloadEvent> {
    int download_id_;
    std::function<void(int, const DownloadEvent&)> callback_;
public:
    DownloadObserverAdapter(int id, std::function<void(int, const DownloadEvent&)> cb)
        : download_id_(id), callback_(std::move(cb)) {}

    void on_update(const DownloadEvent& event) override {
        callback_(download_id_, event);
    }
};

class AppUI {
public:
    explicit AppUI(AppConfig config);
    void run(const std::string& initial_url = "", const std::string& output_dir = ".");

private:
    AppConfig config_;
    std::mutex mutex_;
    ftxui::ScreenInteractive* screen_ = nullptr;

    int next_id_ = 0;
    std::vector<std::unique_ptr<DownloadEntry>> downloads_;
    std::vector<std::thread> download_threads_;
    int selected_ = 0;

    std::string url_input_;
    std::string cfg_connections_;
    std::string cfg_downloads_;
    std::string cfg_retries_;
    std::string cfg_output_dir_;
    int detail_tab_ = 0;
    bool confirming_exit_ = false;
    bool editing_config_ = false;

    void submit_url();
    void start_download(DownloadEntry* entry);
    void on_download_event(int download_id, const DownloadEvent& event);
    void try_start_queued();
    void save_config();
};

#endif //CDOWNLOAD_MANAGER_UI_H
