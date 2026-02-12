#include "ui.h"
#include "download_manager.h"
#include "downloader.h"
#include "structs.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>

using namespace ftxui;
namespace fs = std::filesystem;

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
        case PENDING:  return "PENDENTE";
        case STARTED:  return "INICIANDO";
        case RUNNING:  return "BAIXANDO";
        case FINISHED: return "CONCLUIDO";
        case FAILED:   return "FALHOU";
        default:       return "DESCONHECIDO";
    }
}

AppUI::AppUI(AppConfig config)
    : config_(config)
    , cfg_connections_(std::to_string(config.max_connections))
    , cfg_downloads_(std::to_string(config.max_downloads))
    , cfg_retries_(std::to_string(config.max_retries))
{}

void AppUI::submit_url() {
    std::lock_guard lock(mutex_);
    if (url_input_.empty()) return;

    auto entry = std::make_unique<DownloadEntry>();
    entry->id = next_id_++;
    entry->url = url_input_;
    entry->output_dir = ".";
    entry->status = PENDING;
    downloads_.push_back(std::move(entry));
    selected_ = static_cast<int>(downloads_.size()) - 1;
    url_input_.clear();

    try_start_queued();
}

void AppUI::start_download(DownloadEntry* entry) {
    auto callback = [this](int id, const DownloadEvent& event) {
        on_download_event(id, event);
    };

    int entry_id = entry->id;
    std::string url = entry->url;
    std::string output_dir = entry->output_dir;
    int max_connections = config_.max_connections;

    download_threads_.emplace_back([this, entry, entry_id, url, output_dir, max_connections, callback]() {
        PreDownloadInfo info = PreDownloadInfo::check_info(url, false);

        {
            std::lock_guard lock(mutex_);
            entry->filename = info.filename;
            entry->content_size = info.content_size;
            entry->accept_ranges = info.accept_ranges;
            entry->output_path = (fs::path(output_dir) / info.filename).string();
            entry->status = STARTED;
        }

        if (screen_) screen_->Post(Event::Custom);

        // Pre-allocate file
        {
            std::ofstream file(entry->output_path, std::ios::binary);
            if (info.content_size > 0) {
                file.seekp(static_cast<std::streamoff>(info.content_size) - 1);
                file.write("", 1);
            }
            file.close();
        }

        std::unique_ptr<DefaultDownloader> downloader;
        if (DownloadManager::should_split(info.content_size, info.accept_ranges)) {
            downloader = std::make_unique<ParalellDownloader>(max_connections);
        } else {
            downloader = std::make_unique<SingleDownloader>();
        }

        auto adapter = std::make_unique<DownloadObserverAdapter>(entry_id, callback);
        downloader->add_observer(adapter.get());

        DownloadOptions options{info.url, entry->output_path, info.content_size};
        downloader->download(options);
    });
}

void AppUI::on_download_event(int download_id, const DownloadEvent& event) {
    std::lock_guard lock(mutex_);

    DownloadEntry* entry = nullptr;
    for (auto& d : downloads_) {
        if (d->id == download_id) {
            entry = d.get();
            break;
        }
    }
    if (!entry) return;

    if (event.thread_id >= 0) {
        auto& ts = entry->threads[event.thread_id];
        ts.status = event.status;
        ts.bytes_downloaded = event.bytes_downloaded;
        ts.total_bytes = event.total_bytes;
        ts.elapsed_seconds = event.elapsed_seconds;
    }

    entry->status = event.status;
    entry->elapsed_seconds = event.elapsed_seconds;

    size_t total_downloaded = 0;
    for (const auto& [tid, ts] : entry->threads) {
        total_downloaded += ts.bytes_downloaded;
    }
    entry->bytes_downloaded = total_downloaded;

    if (event.status == FINISHED || event.status == FAILED) {
        try_start_queued();
    }

    if (screen_) screen_->Post(Event::Custom);
}

void AppUI::try_start_queued() {
    int active = 0;
    for (const auto& d : downloads_) {
        if (d->status == STARTED || d->status == RUNNING) {
            active++;
        }
    }

    int max_dl = config_.max_downloads;
    for (auto& d : downloads_) {
        if (active >= max_dl) break;
        if (d->status == PENDING) {
            d->status = STARTED;
            start_download(d.get());
            active++;
        }
    }
}

void AppUI::save_config() {
    try {
        config_.max_connections = std::stoi(cfg_connections_);
    } catch (...) {}
    try {
        config_.max_downloads = std::stoi(cfg_downloads_);
    } catch (...) {}
    try {
        config_.max_retries = std::stoi(cfg_retries_);
    } catch (...) {}
    config_.save();
}

void AppUI::run(const std::string& initial_url, const std::string& output_dir) {
    auto screen = ScreenInteractive::Fullscreen();
    screen_ = &screen;

    if (!initial_url.empty()) {
        auto entry = std::make_unique<DownloadEntry>();
        entry->id = next_id_++;
        entry->url = initial_url;
        entry->output_dir = output_dir;
        entry->status = PENDING;
        downloads_.push_back(std::move(entry));
        try_start_queued();
    }

    // --- Left panel: URL input ---
    auto url_input = Input(&url_input_, "cole a URL aqui...");
    auto url_with_enter = CatchEvent(url_input, [&](Event event) {
        if (event == Event::Return) {
            submit_url();
            return true;
        }
        return false;
    });

    // --- Left panel: Config inputs ---
    auto cfg_conn_input = Input(&cfg_connections_, "8");
    auto cfg_dl_input = Input(&cfg_downloads_, "3");
    auto cfg_ret_input = Input(&cfg_retries_, "3");

    auto config_container = Container::Vertical({
        cfg_conn_input,
        cfg_dl_input,
        cfg_ret_input,
    });

    auto left_panel = Container::Vertical({
        url_with_enter,
        config_container,
    });

    auto left_renderer = Renderer(left_panel, [&] {
        return vbox({
            window(text(" home "), vbox({
                hbox({text(" URL "), url_with_enter->Render() | flex}) | size(HEIGHT, EQUAL, 1),
            })),
            window(text(" configs "), vbox({
                hbox({text(" conexoes:   "), cfg_conn_input->Render() | size(WIDTH, EQUAL, 4)}),
                hbox({text(" downloads:  "), cfg_dl_input->Render() | size(WIDTH, EQUAL, 4)}),
                hbox({text(" tentativas: "), cfg_ret_input->Render() | size(WIDTH, EQUAL, 4)}),
            })),
        }) | size(WIDTH, EQUAL, 30);
    });

    // --- Right panel: toggle tabs ---
    std::vector<std::string> tab_labels = {"progresso", "detalhes"};
    auto tab_toggle = Toggle(&tab_labels, &detail_tab_);

    auto right_panel = Container::Vertical({
        tab_toggle,
    });

    auto right_renderer = Renderer(right_panel, [&] {
        std::lock_guard lock(mutex_);

        Elements download_list;
        for (int i = 0; i < static_cast<int>(downloads_.size()); i++) {
            const auto& d = downloads_[i];
            std::string name = d->filename.empty() ? d->url : d->filename;
            if (name.size() > 40) name = name.substr(0, 37) + "...";

            float progress = 0.0f;
            if (d->content_size > 0) {
                progress = static_cast<float>(d->bytes_downloaded) /
                           static_cast<float>(d->content_size);
            }
            int percent = static_cast<int>(progress * 100);

            auto item = vbox({
                text(" " + name) | bold,
                hbox({
                    text(" "),
                    gauge(progress) | flex,
                    text(" " + std::to_string(percent) + "% "),
                }),
            });

            if (i == selected_) {
                item = item | inverted;
            }
            download_list.push_back(item);
        }

        if (download_list.empty()) {
            download_list.push_back(
                text(" nenhum download") | dim | center
            );
        }

        // Tab content for selected download
        Element tab_content;
        if (selected_ >= 0 && selected_ < static_cast<int>(downloads_.size())) {
            const auto& sel = downloads_[selected_];

            if (detail_tab_ == 0) {
                // Progress tab - per-thread gauges
                Elements thread_rows;
                if (sel->threads.empty()) {
                    thread_rows.push_back(text(" aguardando threads...") | dim);
                } else {
                    for (const auto& [tid, ts] : sel->threads) {
                        float t_progress = 0.0f;
                        if (ts.total_bytes > 0) {
                            t_progress = static_cast<float>(ts.bytes_downloaded) /
                                         static_cast<float>(ts.total_bytes);
                        }
                        int t_percent = static_cast<int>(t_progress * 100);

                        Color bar_color = Color::Blue;
                        if (ts.status == FINISHED) bar_color = Color::Green;
                        else if (ts.status == FAILED) bar_color = Color::Red;

                        thread_rows.push_back(hbox({
                            text(" #" + std::to_string(tid)) | size(WIDTH, EQUAL, 5),
                            gauge(t_progress) | flex | color(bar_color),
                            text(" " + std::to_string(t_percent) + "%") | size(WIDTH, EQUAL, 5),
                            text(" " + format_bytes(ts.bytes_downloaded) + "/" + format_bytes(ts.total_bytes)),
                        }));
                    }
                }
                tab_content = vbox(thread_rows);
            } else {
                // Details tab
                tab_content = vbox({
                    text(" arquivo:       " + sel->filename),
                    text(" url:           " + sel->url),
                    text(" diretorio:     " + sel->output_dir),
                    text(" tamanho:       " + format_bytes(sel->content_size)),
                    text(" accept_ranges: " + std::string(sel->accept_ranges ? "sim" : "nao")),
                    text(" status:        " + status_to_string(sel->status)),
                    text(" tempo:         " + format_time(sel->elapsed_seconds)),
                    text(" baixado:       " + format_bytes(sel->bytes_downloaded)),
                });
            }
        } else {
            tab_content = text(" selecione um download") | dim;
        }

        return window(text(" monitor "), vbox({
            vbox(download_list) | vscroll_indicator | yframe | flex,
            separator(),
            hbox({text(" "), tab_toggle->Render()}),
            separator(),
            tab_content | flex,
        })) | flex;
    });

    // --- Main layout ---
    auto main_container = Container::Horizontal({
        left_renderer,
        right_renderer,
    });

    auto main_renderer = Renderer(main_container, [&] {
        auto content = hbox({
            left_renderer->Render(),
            right_renderer->Render() | flex,
        });

        std::string status_text = " Ctrl+Q: Sair | Enter: Download | Up/Down: Navegar";
        if (confirming_exit_) {
            status_text = " Download em andamento! Sair? (s/n)";
        }

        return vbox({
            text(" cdownload-manager") | bold | hcenter,
            separator(),
            content | flex,
            separator(),
            text(status_text) | bgcolor(Color::Green) | color(Color::White),
        }) | border;
    });

    auto component = CatchEvent(main_renderer, [&](Event event) {
        if (event == Event::Special("\x11")) { // Ctrl+Q
            std::lock_guard lock(mutex_);
            bool has_active = false;
            for (const auto& d : downloads_) {
                if (d->status == STARTED || d->status == RUNNING || d->status == PENDING) {
                    has_active = true;
                    break;
                }
            }
            if (!has_active) {
                save_config();
                screen.Exit();
                return true;
            }
            confirming_exit_ = true;
            return true;
        }

        if (confirming_exit_) {
            if (event == Event::Character('s') || event == Event::Character('S') ||
                event == Event::Character('y') || event == Event::Character('Y')) {
                save_config();
                screen.Exit();
                return true;
            }
            if (event == Event::Character('n') || event == Event::Character('N')) {
                confirming_exit_ = false;
                return true;
            }
            return true;
        }

        if (event == Event::ArrowUp) {
            std::lock_guard lock(mutex_);
            if (selected_ > 0) selected_--;
            return true;
        }
        if (event == Event::ArrowDown) {
            std::lock_guard lock(mutex_);
            if (selected_ < static_cast<int>(downloads_.size()) - 1) selected_++;
            return true;
        }

        return false;
    });

    screen.Loop(component);
    screen_ = nullptr;

    for (auto& t : download_threads_) {
        if (t.joinable()) t.join();
    }
}
