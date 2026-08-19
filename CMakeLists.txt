cmake_minimum_required(VERSION 3.22)
include(FetchContent)

project(
    basil
    VERSION 2026.8.19
    DESCRIPTION "A log aggregation and monitoring solution."
    LANGUAGES C CXX)
# --------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# --------------------------------------------------------
execute_process(COMMAND git describe --tags --always --dirty
    OUTPUT_VARIABLE GIT_REV
    ERROR_QUIET
)
string(STRIP "${GIT_REV}" GIT_REV)
string(TIMESTAMP BUILD_TIME UTC)
configure_file(include/basil/version.h.in ${PROJECT_SOURCE_DIR}/include/basil/version.hpp)
# --------------------------------------------------------
find_package(argparse CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(inja CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(boost_exception CONFIG REQUIRED)

# --------------------------------------------------------
add_subdirectory(gourd)
# --------------------------------------------------------
file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(basil ${SOURCE_LIST})
set_target_properties(basil PROPERTIES LINK_FLAGS_RELEASE -s)
target_link_options(basil PRIVATE -static-libgcc -static-libstdc++)

target_link_libraries(basil gourd
  Boost::exception
  argparse::argparse
  pantor::inja nlohmann_json::nlohmann_json
  spdlog::spdlog_header_only
  OpenSSL::SSL OpenSSL::Crypto
)

target_include_directories(loquat PUBLIC
  ${PROJECT_SOURCE_DIR}/include
)
