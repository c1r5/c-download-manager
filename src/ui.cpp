#include "ui.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <sstream>

using namespace ftxui;

static std::string format_bytes(size_t bytes) {
    std::ostringstream oss;
    if (bytes >= 1024ULL * 1024 * 1024) {
        oss << std::fixed << std::setprecision(1)
            << static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0) << " GB";
    } else if (bytes >= 1024ULL * 1024) {
        oss << std::fixed << std::setprecision(1)
            << static_cast<double>(bytes) / (1024.0 * 1024.0) << " MB";
    } else if (bytes >= 1024) {
        oss << std::fixed << std::setprecision(1)
            << static_cast<double>(bytes) / 1024.0 << " KB";
    } else {
        oss << bytes << " B";
    }
    return oss.str();
}

static std::string format_time(double seconds) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << seconds << "s";
    return oss.str();
}

static std::string status_to_string(DownloadStatus s) {
    switch (s) {
        case PENDING:  return "PENDING";
        case STARTED:  return "STARTED";
        case RUNNING:  return "RUNNING";
        case FINISHED: return "FINISHED";
        case FAILED:   return "FAILED";
        default:       return "UNKNOWN";
    }
}

void DownloadManagerUI::on_update(const DownloadEvent& event) {
    std::lock_guard lock(mutex_);
    status_ = event.status;
    bytes_downloaded_ = event.bytes_downloaded;
    total_bytes_ = event.total_bytes;
    elapsed_seconds_ = event.elapsed_seconds;

    if (screen_) {
        screen_->Post(Event::Custom);
    }
}

void DownloadManagerUI::run(const std::string& filename) {
    auto screen = ScreenInteractive::Fullscreen();
    screen_ = &screen;

    auto renderer = Renderer([&] {
        std::lock_guard lock(mutex_);

        float progress = 0.0f;
        if (total_bytes_ > 0) {
            progress = static_cast<float>(bytes_downloaded_) /
                       static_cast<float>(total_bytes_);
        }
        int percent = static_cast<int>(progress * 100);

        std::string status_str = status_to_string(status_);
        std::string downloaded_str = format_bytes(bytes_downloaded_);
        std::string total_str = format_bytes(total_bytes_);
        std::string time_str = format_time(elapsed_seconds_);

        Elements content;
        content.push_back(text("  cdownload-manager") | bold);
        content.push_back(separator());
        content.push_back(text("  Arquivo: " + filename));
        content.push_back(text("  Status:  " + status_str));
        content.push_back(text(""));
        content.push_back(
            hbox({
                text("  "),
                gauge(progress) | flex,
                text(" " + std::to_string(percent) + "%"),
            })
        );
        content.push_back(
            text("  " + downloaded_str + " / " + total_str + " | " + time_str)
        );
        content.push_back(text(""));

        if (confirming_exit_) {
            content.push_back(text("  Download em andamento!") | bold | color(Color::Yellow));
            content.push_back(text("  Tem certeza que deseja sair? (s/n)") | color(Color::Yellow));
        } else if (status_ == FINISHED) {
            content.push_back(text("  Download concluido!") | bold | color(Color::Green));
            content.push_back(text("  Pressione 'q' para sair"));
        } else if (status_ == FAILED) {
            content.push_back(text("  Download falhou!") | bold | color(Color::Red));
            content.push_back(text("  Pressione 'q' para sair"));
        } else {
            content.push_back(text("  Pressione 'q' para sair"));
        }

        return vbox(content) | border;
    });

    auto component = CatchEvent(renderer, [&](Event event) {
        if (event == Event::Character('q') || event == Event::Character('Q')) {
            std::lock_guard lock(mutex_);
            if (status_ == FINISHED || status_ == FAILED) {
                screen.Exit();
                return true;
            }
            confirming_exit_ = true;
            return true;
        }

        if (confirming_exit_) {
            if (event == Event::Character('s') || event == Event::Character('S') ||
                event == Event::Character('y') || event == Event::Character('Y')) {
                screen.Exit();
                return true;
            }
            if (event == Event::Character('n') || event == Event::Character('N')) {
                std::lock_guard lock(mutex_);
                confirming_exit_ = false;
                return true;
            }
        }

        return false;
    });

    screen.Loop(component);
    screen_ = nullptr;
}
