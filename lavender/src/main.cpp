#include "lavender/application.hpp"

#include <cstdlib>

#include <boost/exception/diagnostic_information.hpp>

#include <spdlog/spdlog.h>

int main(int argc, char** argv) {
  lavender::Application app;
  try {
    return app.launch(argc, argv);
  } catch (...) {
    spdlog::error("{}", boost::current_exception_diagnostic_information());
  }
  return EXIT_FAILURE;
}
