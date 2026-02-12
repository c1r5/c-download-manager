#ifndef CDOWNLOAD_MANAGER_UI_H
#define CDOWNLOAD_MANAGER_UI_H

#include "observer.h"
#include "structs.h"
#include <map>
#include <mutex>
#include <string>

namespace ftxui {
class ScreenInteractive;
}

struct ThreadState {
    DownloadStatus status = PENDING;
    size_t bytes_downloaded = 0;
    size_t total_bytes = 0;
    double elapsed_seconds = 0.0;
};

class DownloadManagerUI : public IObserver<DownloadEvent> {
public:
    DownloadManagerUI() = default;
    ~DownloadManagerUI() = default;

    void on_update(const DownloadEvent& event) override;
    void run(const std::string& filename);

private:
    std::mutex mutex_;
    DownloadStatus status_ = PENDING;
    size_t total_bytes_ = 0;
    double elapsed_seconds_ = 0.0;
    bool confirming_exit_ = false;

    std::map<int, ThreadState> threads_;

    ftxui::ScreenInteractive* screen_ = nullptr;
};

#endif //CDOWNLOAD_MANAGER_UI_H
