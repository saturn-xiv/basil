#pragma once

#include "lavender/open-search.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include <nlohmann/json.hpp>

namespace lavender {
namespace logging {
namespace filesystem {
struct Message {
  std::string host;
  std::string file;
  std::string line;
  std::chrono::time_point<std::chrono::high_resolution_clock> created_at;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Message, host, file, line, created_at)

class File {
 public:
  File(const std::filesystem::path& file);
  ~File();

  std::string read();

 private:
  std::filesystem::path _file;
  long _pos;
  std::mutex _mutex;
};

class Watcher {
 public:
  Watcher(std::shared_ptr<lavender::OpenSearch> search,
          const std::filesystem::path& path);
  ~Watcher();

  void watch();
  void sync(const std::filesystem::path& file);

 private:
  std::shared_ptr<lavender::OpenSearch> _search;
  std::mutex _mutex;
  std::map<std::filesystem::path, std::shared_ptr<File>> _items;
  std::filesystem::path _root;

  int _file;
  int _watcher;
};

}  // namespace filesystem
}  // namespace logging
}  // namespace lavender

namespace nlohmann {
template <typename Clock, typename Duration>
struct adl_serializer<std::chrono::time_point<Clock, Duration>> {
  static void to_json(nlohmann::json& j,
                      const std::chrono::time_point<Clock, Duration>& o) {
    // j = std::chrono::duration_cast<std::chrono::nanoseconds>(
    //         o.time_since_epoch())
    //         .count();
    // "2022-06-15T10:12:52.382719622Z"
    j = std::format("{:%FT%T%z}", o);
  }

  static void from_json(const nlohmann::json& j,
                        std::chrono::time_point<Clock, Duration>& o) {
    // std::chrono::nanoseconds dur(j.get<int64_t>());
    // o = dur;

    const std::string s = j.get<std::string>();
    std::istringstream in{s};
    in >> std::chrono::parse("%FT%T%z", o);
    if (in.fail()) {
      BOOST_LOG_TRIVIAL(error) << "failed to parse " << s;
    }
  }
};
}  // namespace nlohmann
