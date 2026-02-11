cmake_minimum_required(VERSION 3.16)
project(cdownload-manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile commands" FORCE)
set(CPR_USE_SYSTEM_CURL ON CACHE BOOL "Use system curl")

add_subdirectory(libs/external/cpr)
add_subdirectory(libs/external/argparse)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADERS ${CMAKE_SOURCE_DIR}/src/*.hpp)

add_executable(
  app
  ${SOURCES}
)

target_link_libraries(
  app
  PRIVATE
  cpr::cpr
  argparse::argparse
)

