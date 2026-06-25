#include "lavender/application.hpp"

#include <cstdlib>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/trivial.hpp>

int main(int argc, char** argv) {
  lavender::Application app;
  try {
    return app.launch(argc, argv);
  } catch (...) {
    BOOST_LOG_TRIVIAL(error)
        << boost::current_exception_diagnostic_information();
  }
  return EXIT_FAILURE;
}
