#ifndef CDOWNLOAD_MANAGER_DOWNLOAD_MANAGER_H
#define CDOWNLOAD_MANAGER_DOWNLOAD_MANAGER_H

#include <cstddef>
#include <string> // IWYU pragma: keep
#include <vector> // IWYU pragma: keep
#include <cpr/range.h>

class DownloadManager {
public:
	DownloadManager();
	static int run(int argc, char *argv[]);
	static bool should_split(size_t size, bool accept_ranges);
	~DownloadManager();
};


#endif // !DOWNLOAD_MANAGER_HPP
