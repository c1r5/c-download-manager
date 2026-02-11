#ifndef DOWNLOAD_MANAGER_HPP
#define DOWNLOAD_MANAGER_HPP

#include <sstream>
#include <string>
#include <vector>

#include "cpr/range.h"


enum DownloadStatus { PENDING, STARTED, RUNNING, FINISHED, FAILED };

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

struct PreDownloadInfo {
	bool accept_ranges;
	size_t content_size;
	std::string url;

	static PreDownloadInfo check_info(const std::string &url);
};

inline std::ostream& operator<<(std::ostream& os, const PreDownloadInfo& info) {
	os << "PreDownloadInfo{"
	   << "accept_ranges=" << std::boolalpha << info.accept_ranges
	   << ", content_size=" << info.content_size
	   << ", url=" << info.url
	   << "}";
	return os;
}

class DownloadManager {
	static bool should_split(size_t size, bool accept_ranges);
public:
	DownloadManager();
	static std::vector<cpr::Range> split_ranges(size_t total, size_t parts);
	static int run(int argc, char *argv[]);
	~DownloadManager();
};

struct DownloadOptions {
	const std::string &url;
	const std::string &out;
	const size_t &c_size;
};
class DefaultDownloader {
	public:
	virtual ~DefaultDownloader() = default;
	virtual void download(const DownloadOptions &options) = 0;
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

#endif // !DOWNLOAD_MANAGER_HPP
