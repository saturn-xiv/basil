cmake_minimum_required(VERSION 3.28)
include(FetchContent)

project(
  lavender
  VERSION 2026.6.25
  DESCRIPTION "A logging & SNMP crawler for basil."
  LANGUAGES CXX
)
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_LINKER_TYPE MOLD)
# -----------------------------------------------------------------------------
execute_process(COMMAND git describe --tags --always --dirty
    OUTPUT_VARIABLE GIT_REV
    ERROR_QUIET
)
string(STRIP "${GIT_REV}" GIT_REV)
string(TIMESTAMP BUILD_TIME UTC)
configure_file(include/lavender/version.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/lavender/version.hpp)
# -----------------------------------------------------------------------------
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED)
find_package(CURL REQUIRED)
# -----------------------------------------------------------------------------
# https://github.com/libcpr/cpr
FetchContent_Declare(cpr 
  GIT_REPOSITORY "https://github.com/libcpr/cpr.git"
  GIT_TAG "1.14.2"
)
# https://github.com/gabime/spdlog
FetchContent_Declare(spdlog
  GIT_REPOSITORY  "https://github.com/gabime/spdlog.git"
  GIT_TAG         "v1.17.0"
)
# https://github.com/pantor/inja
FetchContent_Declare(inja
  GIT_REPOSITORY  "https://github.com/pantor/inja.git"
  GIT_TAG         "v3.5.0"
)
# https://github.com/p-ranav/argparse
FetchContent_Declare(argparse
  GIT_REPOSITORY  "https://github.com/p-ranav/argparse.git"
  GIT_TAG         "v3.2"
)
FetchContent_MakeAvailable(argparse spdlog inja cpr)
# -----------------------------------------------------------------------------
file(GLOB LAVENDER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(lavender ${LAVENDER_SOURCES})
target_link_libraries(lavender PRIVATE Boost::headers CURL::libcurl
  argparse::argparse spdlog::spdlog_header_only pantor::inja cpr::cpr
)
target_link_options(lavender PRIVATE -static -static-libgcc -static-libstdc++ -Wl,--build-id=sha1)
set_target_properties(lavender PROPERTIES LINK_FLAGS_RELEASE -s)    

target_include_directories(lavender PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
