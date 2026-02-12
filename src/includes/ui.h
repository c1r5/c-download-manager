#ifndef CDOWNLOAD_MANAGER_UI_H
#define CDOWNLOAD_MANAGER_UI_H

#include "observer.h"
#include "structs.h"
#include <mutex>
#include <string>

namespace ftxui {
class ScreenInteractive;
}

class DownloadManagerUI : public IObserver<DownloadEvent> {
public:
    DownloadManagerUI() = default;
    ~DownloadManagerUI() = default;

    void on_update(const DownloadEvent& event) override;
    void run(const std::string& filename);

private:
    std::mutex mutex_;
    DownloadStatus status_ = PENDING;
    size_t bytes_downloaded_ = 0;
    size_t total_bytes_ = 0;
    double elapsed_seconds_ = 0.0;
    bool confirming_exit_ = false;

    ftxui::ScreenInteractive* screen_ = nullptr;
};

#endif //CDOWNLOAD_MANAGER_UI_H
