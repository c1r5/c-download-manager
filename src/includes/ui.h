#ifndef CDOWNLOAD_MANAGER_UI_H
#define CDOWNLOAD_MANAGER_UI_H

#include "observer.h"
#include "structs.h"

class DownloadManagerUI : public IObserver<DownloadEvent> {
public:
    DownloadManagerUI() = default;
    ~DownloadManagerUI() = default;

    void on_update(const DownloadEvent& event) override;
};

#endif //CDOWNLOAD_MANAGER_UI_H
