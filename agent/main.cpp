#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>

namespace basil {
class Application {
 public:
  Application(int argc, char* argv[]) {
    std::vector<std::string> snmp_hosts;

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("help,h", "Produce help message")(
        "version,v", "Print version")("debug,d", "Run on debug mode")(
        "interval,i",
        boost::program_options::value<uint16_t>()->default_value(MIN_INTERVAL),
        "Time interval in seconds")(
        "host,H",
        boost::program_options::value<std::vector<std::string>>(&snmp_hosts),
        "SNMP hosts")(
        "config,c",
        boost::program_options::value<std::string>()->default_value(
            "config.ini"),
        "Configuration file");

    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return;
    }
    if (vm.count("version")) {
      std::cout << "v2026.05.28" << std::endl;
      return;
    }
    {
      boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                          (vm.count("debug")
                                               ? boost::log::trivial::debug
                                               : boost::log::trivial::info));
      BOOST_LOG_TRIVIAL(debug) << "run on debug mode";
    }

    const auto config_file = vm["config"].as<std::string>();
    BOOST_LOG_TRIVIAL(debug) << "load configuration from " << config_file;

    for (const auto& it : snmp_hosts) {
      BOOST_LOG_TRIVIAL(info) << "found SNMP host " << it;
    }

    auto interval = vm["interval"].as<uint16_t>();
    if (interval < MIN_INTERVAL) {
      interval = MIN_INTERVAL;
    }
    BOOST_LOG_TRIVIAL(debug)
        << "collect SNMP data with time interval(" << interval << " seconds)";
  }

 private:
  static constexpr const uint16_t MIN_INTERVAL = 5;
};
}  // namespace basil

int main(int argc, char* argv[]) {
  try {
    basil::Application app(argc, argv);
  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
