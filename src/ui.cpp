#include "ui.h"
#include <iostream>

void DownloadManagerUI::on_update(const DownloadEvent& event) {
	std::cout << "[" << event.status << "] "
	          << event.bytes_downloaded << "/" << event.total_bytes
	          << " bytes | " << event.elapsed_seconds << "s"
	          << std::endl;
}
