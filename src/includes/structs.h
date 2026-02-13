//
//

#ifndef CDOWNLOAD_MANAGER_STRUCTS_H
#define CDOWNLOAD_MANAGER_STRUCTS_H

#include <map>
#include <ostream>
#include <string>

enum DownloadStatus { PENDING, STARTED, RUNNING, FINISHED, FAILED };

struct DownloadOptions {
    const std::string &url;
    const std::string &out;
    const size_t &c_size;
};

struct PreDownloadInfo {
    bool accept_ranges;
    size_t content_size;
    std::string url;
    std::string filename;

    static PreDownloadInfo check_info(const std::string &url, const bool &header_only = false);
};

inline std::ostream& operator<<(std::ostream& os, const DownloadStatus status) {
    switch (status) {
        case PENDING:  return os << "PENDING";
        case STARTED:  return os << "STARTED";
        case RUNNING:  return os << "RUNNING";
        case FINISHED: return os << "FINISHED";
        case FAILED:   return os << "FAILED";
        default:       return os << "UNKNOWN";
    }
}

inline std::ostream& operator<<(std::ostream& os, const PreDownloadInfo& info) {
    os << "PreDownloadInfo{"
       << "accept_ranges=" << std::boolalpha << info.accept_ranges
       << ", content_size=" << info.content_size
       << ", url=" << info.url
       << ", filename=" << info.filename
       << "}";
    return os;
}

struct DownloadEvent {
    DownloadStatus status;
    size_t bytes_downloaded;
    size_t total_bytes;
    double elapsed_seconds;
    int thread_id = -1;
};

struct ThreadState {
    DownloadStatus status = PENDING;
    size_t bytes_downloaded = 0;
    size_t total_bytes = 0;
    double elapsed_seconds = 0.0;
};

struct DownloadEntry {
    int id;
    std::string url;
    std::string filename;
    std::string output_dir;
    std::string output_path;
    bool accept_ranges = false;
    size_t content_size = 0;
    DownloadStatus status = PENDING;
    size_t bytes_downloaded = 0;
    double elapsed_seconds = 0.0;
    std::map<int, ThreadState> threads;
};

#endif //CDOWNLOAD_MANAGER_STRUCTS_H
