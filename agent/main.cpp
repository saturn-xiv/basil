#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <iostream>
#include <list>
#include <regex>
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
#include <boost/process.hpp>
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
class Pod {
 public:
  Pod(const std::string& namespace_, const std::string& name,
      const std::string& line)
      : _namespace(namespace_), _name(name) {
    std::regex pattern(R"(\[([\w\/-]+)\] ([\w:.-]+) (.+))");
    std::smatch matches;
    if (std::regex_search(line, matches, pattern)) {
      std::cout << " matched " << matches.size() << std::endl;
      for (const auto& it : matches) {
        std::cout << it << std::endl;
      }

      if (matches.size() == 4) {
        this->_create_at = matches[2];
        this->_message = matches[3];
        return;
      }
    }
    throw std::invalid_argument("unmatch k8s log message");
  }
  std::string id() const {
    return std::format("{}.{}", this->_name, this->_create_at);
  }
  void dump(boost::json::object& body) const {
    body["namespace"] = this->_namespace;
    body["name"] = this->_name;
    body["createAt"] = this->_create_at;
    body["message"] = this->_message;
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
      properties["message"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "date_nanos";
      properties["createdAt"] = it;
    }
  }

 private:
  std::string _namespace;
  std::string _name;
  std::string _create_at;
  std::string _message;
};
}  // namespace k8s
struct Snmp {
  Snmp(const std::string& host) {
    // TODO
  }
  void dump(boost::json::object& body) const {
    // TODO
  }
  std::string name;
  std::string ipv4;
  size_t memory_usage;
  size_t memory_total;
  size_t cpu_usage;

  static void properties(boost::json::object& properties) {
    {
      boost::json::object it;
      it["type"] = "text";
      properties["name"] = it;
    }
    {
      boost::json::object it;
      it["type"] = "text";
      properties["ipv4"] = it;
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

  // https://docs.opensearch.org/latest/api-reference/document-apis/bulk/
  template <typename T>
  void save(const std::vector<T> items) {
    if (items.empty()) {
      return;
    }

    const std::string name = this->index_name<T>();
    BOOST_LOG_TRIVIAL(info)
        << "save " << items.size() << " documents into " << name;

    std::stringstream body;
    for (const auto& it : items) {
      boost::json::object header;
      {
        boost::json::object index;
        index["_index"] = name;
        index["_id"] = it.id();

        header["index"] = index;
      }
      boost::json::object data;
      it.dump(data);
      body << boost::json::serialize(header) << "\n"
           << boost::json::serialize(data) << "\n";
    }

    const std::string body_s = body.str();

    const std::string url = std::format("{}/_bulk", this->_endpoint);

    std::stringstream res;
    curlpp::Easy req;
    req.setOpt<curlpp::options::Url>(url);
    req.setOpt(new cURLpp::options::HttpPost);
    req.setOpt(new curlpp::options::PostFields(body_s));
    req.setOpt(new curlpp::options::PostFieldSize(body_s.length()));
    req.setOpt(new curlpp::options::HttpHeader(x_ndjson_headers()));
    req.setOpt(new curlpp::options::WriteStream(&res));
    req.perform();

    const auto status = curlpp::infos::ResponseCode::get(req);
    BOOST_LOG_TRIVIAL(debug) << status << " " << res.str();
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
  inline std::list<std::string> x_ndjson_headers() {
    std::list<std::string> items;
    items.push_back("Content-Type: application/x-ndjson");
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

void k8s_logs(std::shared_ptr<OpenSearch> search, const std::string& namespace_,
              const std::chrono::seconds& interval) {
  auto since = std::chrono::utc_clock::now();

  while (gl_keep_running) {
    std::this_thread::sleep_for(interval);
    // RFC3339
    const std::string since_s = std::format(
        "{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(since));

    try {
      std::vector<std::string> pods;
      {
        BOOST_LOG_TRIVIAL(debug) << "get k8s pods for " << namespace_;

        const std::string command =
            std::format(R"(kubectl get pods -n {} -o json)", namespace_);
        BOOST_LOG_TRIVIAL(debug) << "run: " << command;
        boost::process::ipstream pipe_stream;
        boost::process::child child(command,
                                    boost::process::std_out > pipe_stream);
        child.wait();

        const auto root = boost::json::parse(pipe_stream);
        for (auto const& item : root.at("items").as_array()) {
          const auto metadata = item.at("metadata").as_object();
          const std::string pod = metadata.at("name").as_string().c_str();
          BOOST_LOG_TRIVIAL(debug) << "found pod " << namespace_ << "/" << pod;
          pods.push_back(pod);
        }
      }

      since = std::chrono::utc_clock::now();
      std::vector<k8s::Pod> logs;
      for (const auto& pod : pods) {
        BOOST_LOG_TRIVIAL(debug) << "get k8s logs for " << pod << "/"
                                 << namespace_ << ") since " << since_s;
        const std::string command = std::format(
            R"(kubectl logs {} -n {} --all-pods --timestamps --all-containers --since-time={})",
            pod, namespace_, since_s);
        BOOST_LOG_TRIVIAL(debug) << "run: " << command;

        boost::process::ipstream pipe_stream;
        boost::process::child child(command,
                                    boost::process::std_out > pipe_stream);

        std::string line;
        while (std::getline(pipe_stream, line) && !line.empty()) {
          boost::trim(line);
          BOOST_LOG_TRIVIAL(debug) << "receive line: " << line;
          logs.emplace_back(namespace_, pod, line);
        }

        child.wait();
      }

      search->save(logs);
    } catch (const std::exception& e) {
      BOOST_LOG_TRIVIAL(error) << e.what();
    }
  }
}

void k8s_pod_logs(std::shared_ptr<OpenSearch> search) {
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
    std::vector<std::string> k8s_namespaces;

    boost::program_options::options_description desc("Generic options");

    desc.add_options()("help,h", "Produce help message")(
        "version,v", "Print version")("debug,d", "Run on debug mode")(
        "interval,i",
        boost::program_options::value<uint16_t>()->default_value(MIN_INTERVAL),
        "Time interval in seconds")(
        "host,H",
        boost::program_options::value<std::vector<std::string>>(&snmp_hosts),
        "SNMP hosts")("kubernetes,K",
                      boost::program_options::value<std::vector<std::string>>(
                          &k8s_namespaces),
                      "Collect logs for kubernetes namespaces")(
        "config,c",
        boost::program_options::value<std::string>()->default_value(
            "config.ini"),
        "Configuration file");

    boost::program_options::variables_map vm;
    boost::program_options::store(
        boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
      std::cout << "A log aggregation and monitoring solution.\n\n"
                << desc << "\nhttps://github.com/saturn-xiv/basil" << std::endl;
      return;
    }
    if (vm.count("version")) {
      std::cout << BASIL_VERSION << "(" << BASIL_BUILD_TIME << ")" << std::endl;
      return;
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

    const auto ttl = std::chrono::seconds(interval);
    for (const auto& host : snmp_hosts) {
      BOOST_LOG_TRIVIAL(info) << "start a watcher for SNMP host " << host;
      this->_workers.emplace_back(snmp_collector, search, host, ttl);
    }
    for (const auto& namespace_ : k8s_namespaces) {
      BOOST_LOG_TRIVIAL(info)
          << "start a watcher for kubernetes resource" << namespace_;
      this->_workers.emplace_back(k8s_logs, search, namespace_, ttl);
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
  {
    basil::k8s::Pod pod(
        "testing", "hi",
        R"([pod/post-install-sl9xp-s54dj/post-install] 2026-05-24T04:46:47.079665727Z Generating optimized autoload files)");
    boost::json::object body;
    pod.dump(body);
    std::cout << pod.id() << ": " << boost::json::serialize(body) << std::endl;
  }

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
