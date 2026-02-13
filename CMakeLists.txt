cmake_minimum_required(VERSION 3.16)
project(cdownload-manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands" FORCE)
set(CPR_USE_SYSTEM_CURL ON CACHE BOOL "Use system curl")

include(FetchContent)
FetchContent_Declare(
  argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse.git
  GIT_TAG v3.2
)

FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 1.11.1
)

FetchContent_Declare(
  ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui.git
  GIT_TAG v5.0.0
)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(argparse cpr ftxui spdlog)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(app ${SOURCES})

target_include_directories(app PRIVATE ${CMAKE_SOURCE_DIR}/src/includes)

target_link_libraries(app
  PRIVATE
  cpr
  argparse
  ftxui::screen
  ftxui::dom
  ftxui::component
  spdlog::spdlog
)

