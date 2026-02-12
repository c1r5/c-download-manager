#ifndef CDOWNLOAD_MANAGER_DOWNLOADER_H
#define CDOWNLOAD_MANAGER_DOWNLOADER_H

#include "observer.h"
#include "structs.h"
#include <algorithm>
#include <vector>

class DefaultDownloader : public IProducer<DownloadEvent> {
    std::vector<IObserver<DownloadEvent>*> observers;
public:
    virtual ~DefaultDownloader() = default;
    virtual void download(const DownloadOptions &options) = 0;

    void add_observer(IObserver<DownloadEvent>* listener) override {
        observers.push_back(listener);
    }

    void remove_observer(IObserver<DownloadEvent>* listener) override {
        observers.erase(
            std::remove(observers.begin(), observers.end(), listener),
            observers.end()
        );
    }

    void emit(const DownloadEvent& event) override {
        for (auto* obs : observers) {
            obs->on_update(event);
        }
    }
};

class SingleDownloader : public DefaultDownloader {
public:
    void download(const DownloadOptions &options) override;
};

class ParalellDownloader : public DefaultDownloader {
    int thread_count;
public:
    ParalellDownloader(const int threads): thread_count(threads) {};
    void download(const DownloadOptions &options) override;
};

#endif //CDOWNLOAD_MANAGER_DOWNLOADER_H
