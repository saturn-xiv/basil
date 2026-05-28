#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <sys/inotify.h>

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/type_index.hpp>

#include <curlpp/Easy.hpp>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

std::atomic<bool> gl_keep_running(true);
void signal_handler(int signal) {
  BOOST_LOG_TRIVIAL(warning) << "receive signal " << strsignal(signal);
  if (signal == SIGINT || signal == SIGTERM) {
    gl_keep_running = false;
  }
}

namespace basil {
namespace k8s {
struct Pod {
  static void document(boost::json::object& body, const std::string& line) {
    // TODO
  }
  static void properties(boost::json::object& properties) {
    {
      boost::json::object it;
      it["type"] = "text";
      properties["namespace"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "text";
      properties["name"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "text";
      properties["level"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "text";
      properties["message"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "date_nanos";
      properties["createdAt"] = it;
    }
  }
};
}  // namespace k8s
struct Snmp {
  static void document(boost::json::object& body, const std::string& host) {
    // TODO
  }
  static void properties(boost::json::object& properties) {
    {
      boost::json::object it;
      it["type"] = "text";
      properties["host"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "integer";
      properties["cpu"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "integer";
      properties["memoryUsage"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "integer";
      properties["memoryTotal"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "date";
      properties["createdAt"] = it;
    }
  }
};

class OpenSearch {
 public:
  OpenSearch(const std::string& endpoint,
             const boost::optional<std::string> namespace_)
      : _endpoint(endpoint), _namespace(namespace_) {
    BOOST_LOG_TRIVIAL(debug) << "open OpenSearch " << endpoint
                             << " with namespace " << namespace_.value_or("''");
  }

  // https://docs.opensearch.org/latest/api-reference/index-apis/exists/
  template <typename T>
  bool index_exists() {
    const std::string name = this->index_name<T>();
    BOOST_LOG_TRIVIAL(debug) << "index-exists " << name;
    const std::string url = std::format("{}/{}", this->_endpoint, name);

    std::stringstream res;
    curlpp::Easy req;
    req.setOpt<curlpp::options::Url>(url);
    req.setOpt(new cURLpp::options::NoBody(true));
    req.setOpt(new cURLpp::options::HttpGet(false));
    req.setOpt(new cURLpp::options::CustomRequest("HEAD"));
    req.setOpt(new curlpp::options::WriteStream(&res));
    req.perform();

    const auto status = curlpp::infos::ResponseCode::get(req);
    BOOST_LOG_TRIVIAL(debug) << status << " " << res.str();
    return status == 200;
  }

  // https://docs.opensearch.org/latest/api-reference/index-apis/create-index/
  // https://docs.opensearch.org/latest/mappings/supported-field-types/index/
  template <typename T>
  void create_index() {
    boost::json::object body;
    {
      boost::json::object properties;
      T::properties(properties);

      boost::json::object mappings;
      mappings["properties"] = properties;
      body["mappings"] = mappings;
    }
    const std::string body_s = boost::json::serialize(body);

    const std::string name = this->index_name<T>();
    BOOST_LOG_TRIVIAL(warning) << "create index " << name << " " << body_s;
    const std::string url = std::format("{}/{}", this->_endpoint, name);

    std::stringstream res;
    curlpp::Easy req;
    req.setOpt<curlpp::options::Url>(url);
    req.setOpt(new cURLpp::options::CustomRequest("PUT"));
    req.setOpt(new curlpp::options::PostFields(body_s));
    req.setOpt(new curlpp::options::PostFieldSize(body_s.length()));
    req.setOpt(new curlpp::options::HttpHeader(json_headers()));
    req.setOpt(new curlpp::options::WriteStream(&res));
    req.perform();

    const auto status = curlpp::infos::ResponseCode::get(req);
    BOOST_LOG_TRIVIAL(debug) << status << " " << res.str();
  }

 private:
  inline std::list<std::string> json_headers() {
    std::list<std::string> items;
    items.push_back("Content-Type: application/json");
    return items;
  }
  template <typename T>
  std::string index_name() {
    std::string name = boost::typeindex::type_id<T>().pretty_name();
    boost::algorithm::replace_all(name, "::", "-");
    boost::algorithm::to_lower(name);

    std::stringstream ss;
    ss << this->_namespace.value_or("") << "." << name;
    return ss.str();
  }

  std::string _endpoint;
  boost::optional<std::string> _namespace;
};

void k8s_pod_logs_watcher(std::shared_ptr<OpenSearch> search) {
  const std::filesystem::path root = "/var/log/pods";
  while (gl_keep_running) {
    BOOST_LOG_TRIVIAL(debug)
        << "watch k8s pod logs on folder " << root.string();
    try {
      // TODO
    } catch (const std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
    }
    std::this_thread::sleep_for(std::chrono::minutes(1));
  }
  // TODO
}

void snmp_collector(std::shared_ptr<OpenSearch> search, const std::string& host,
                    const std::chrono::seconds& interval) {
  while (gl_keep_running) {
    BOOST_LOG_TRIVIAL(debug) << "collect SNMP from " << host;
    try {
      // TODO
    } catch (const std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
    }
    std::this_thread::sleep_for(interval);
  }
}

class Application {
 public:
  Application(int argc, char* argv[]) : _workers() {
    std::vector<std::string> snmp_hosts;

    boost::program_options::options_description desc("Allowed options");

    desc.add_options()("help,h", "Produce help message")(
        "version,v", "Print version")("debug,d", "Run on debug mode")(
        "interval,i",
        boost::program_options::value<uint16_t>()->default_value(MIN_INTERVAL),
        "Time interval in seconds")(
        "host,H",
        boost::program_options::value<std::vector<std::string>>(&snmp_hosts),
        "SNMP hosts")("kubernetes,K", "Listen on kubernetes pod logs")(
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
      std::cout << BASIL_VERSION << "(" << BASIL_BUILD_TIME << ")" << std::endl;
      return;
    }
    bool kubernetes = false;
    if (vm.count("kubernetes")) {
      kubernetes = true;
    }
    {
      boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                          (vm.count("debug")
                                               ? boost::log::trivial::debug
                                               : boost::log::trivial::info));
      BOOST_LOG_TRIVIAL(debug) << "run on debug mode";
    }

    boost::property_tree::ptree config;
    {
      const auto file = vm["config"].as<std::string>();
      BOOST_LOG_TRIVIAL(debug) << "load configuration from " << file;
      boost::property_tree::ini_parser::read_ini(file, config);
    }

    for (const auto& it : snmp_hosts) {
      BOOST_LOG_TRIVIAL(info) << "found SNMP host " << it;
    }

    auto interval = vm["interval"].as<uint16_t>();
    if (interval < MIN_INTERVAL) {
      interval = MIN_INTERVAL;
    }
    BOOST_LOG_TRIVIAL(debug)
        << "collect SNMP data with time interval(" << interval << " seconds)";

    std::shared_ptr<OpenSearch> search = std::make_shared<OpenSearch>(
        config.get<std::string>("opensearch.endpoint", "http://127.0.0.1:9200"),
        config.get_optional<std::string>("opensearch.namespace"));
    {
      if (!search->index_exists<Snmp>()) {
        search->create_index<Snmp>();
      }
      if (!search->index_exists<k8s::Pod>()) {
        search->create_index<k8s::Pod>();
      }
    }

    {
      const auto ttl = std::chrono::seconds(interval);
      for (const auto& host : snmp_hosts) {
        BOOST_LOG_TRIVIAL(info) << "start a watcher for SNMP host " << host;
        this->_workers.emplace_back(snmp_collector, search, host, ttl);
      }
    }

    if (kubernetes) {
      BOOST_LOG_TRIVIAL(info)
          << "start a watcher for kubernetes pod logs folder";
      this->_workers.emplace_back(k8s_pod_logs_watcher, search);
    }
  }

  void start() {
    for (auto& it : this->_workers) {
      if (it.joinable()) {
        it.join();
      }
    }
  }

 private:
  std::vector<std::thread> _workers;

  static constexpr const uint16_t MIN_INTERVAL = 5;
};
}  // namespace basil

int main(int argc, char* argv[]) {
  try {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    basil::Application app(argc, argv);
    app.start();
  } catch (const std::exception& e) {
    BOOST_LOG_TRIVIAL(error) << e.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
