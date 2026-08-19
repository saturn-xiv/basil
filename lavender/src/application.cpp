#include "lavender/application.hpp"
#include "lavender/logging.hpp"
#include "lavender/open-search.hpp"
#include "lavender/version.hpp"

#include <condition_variable>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <argparse/argparse.hpp>

int lavender::Application::launch(int argc, char** argv) const {
  const std::string version =
      std::format("{}({})", lavender::GIT_VERSION, lavender::BUILD_TIME);
  argparse::ArgumentParser program(lavender::PROJECT_NAME, version);
  program.add_description(lavender::PROJECT_DESCRIPTION);
  program.add_epilog(lavender::PROJECT_HOME);

  program.add_argument("-d", "--debug")
      .help("run on debug mode")
      .default_value(false)
      .implicit_value(true);
  program.add_argument("-c", "--config")
      .help("load configuration from")
      .default_value("config.toml")
      .implicit_value(true);

  argparse::ArgumentParser watch_command("watch");
  watch_command.add_description("watch logging files");
  watch_command.add_argument("-s", "--stdin")
      .help("reading from stdin")
      .default_value(false)
      .implicit_value(true);
  watch_command.add_argument("-f", "--folder")
      .nargs(argparse::nargs_pattern::any)
      .help("files to watch");

  argparse::ArgumentParser snmp_command("snmp");
  snmp_command.add_description("collect SNMP information");
  snmp_command.add_argument("-i", "--interval")
      .default_value(5)
      .help("seconds")
      .scan<'i', int>();
  snmp_command.add_argument("-H", "--host")
      .nargs(argparse::nargs_pattern::any)
      .help("snmp hosts to clawer");

  program.add_subparser(watch_command);
  program.add_subparser(snmp_command);

  program.parse_args(argc, argv);

  spdlog::set_level(program.get<bool>("debug") ? spdlog::level::debug
                                               : spdlog::level::info);
  spdlog::debug("runing on debug mode");

  boost::property_tree::ptree tree;
  {
    const std::string config_file = program.get<std::string>("config");
    spdlog::debug("load configuration from file {}", config_file);
    boost::property_tree::ini_parser::read_ini(config_file, tree);
  }
  std::string opensearch_url = tree.get<std::string>("opensearch.url");
  std::string opensearch_namespace =
      tree.get<std::string>("opensearch.namespace");
  auto search = std::make_shared<lavender::OpenSearch>(opensearch_url,
                                                       opensearch_namespace);
  if (!search->index_exists<lavender::logging::filesystem::Message>()) {
    search->create_index<lavender::logging::filesystem::Message>(
        R"(
{
  "host": {
    "type": "text"
  },
  "file": {
    "type": "text"
  },
  "line": {
    "type": "text"
  },
  "created_at": {
    "type": "date_nanos",
    "format": "strict_date_optional_time_nanos"
  }
}
)"_json);
  }

  std::vector<std::thread> pool;

  if (program.present("--folder")) {
    const auto folders = program.get<std::vector<std::string>>("--folders");
    for (const auto& folder : folders) {
      pool.emplace_back([search, &folder] {
        try {
          lavender::logging::filesystem::Watcher it(search, folder);
          for (;;) {
            it.watch();
          }
        } catch (...) {
          spdlog::error("{}",
                        boost::current_exception_diagnostic_information());
        }
      });
    }
  }

  for (auto& it : pool) {
    it.join();
  }
  spdlog::info("done.");

  return EXIT_SUCCESS;
}
