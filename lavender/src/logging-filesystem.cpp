#include "lavender/logging.hpp"

#include <sys/inotify.h>
#include <sys/types.h>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>

lavender::logging::filesystem::File::File(const std::filesystem::path& file)
    : _file(file) {
  BOOST_LOG_TRIVIAL(debug) << "found file " << this->_file.string();
  {
    auto fd = fopen(this->_file.c_str(), "r");
    if (fd == nullptr) {
      BOOST_LOG_TRIVIAL(error) << std::strerror(errno);
      return;
    }
    fseek(fd, 0, SEEK_END);
    this->_pos = ftell(fd);
    fclose(fd);
  }
}
lavender::logging::filesystem::File::~File() {}

std::string lavender::logging::filesystem::File::read() {
  std::lock_guard<std::mutex> lock(this->_mutex);

  std::stringstream ss;
  {
    auto fd = fopen(this->_file.c_str(), "r");
    if (fd == nullptr) {
      BOOST_LOG_TRIVIAL(error) << std::strerror(errno);
      return "";
    }
    if (fseek(fd, this->_pos, SEEK_SET) != 0) {
      BOOST_LOG_TRIVIAL(error) << "invalid postion " << this->_pos;
      fclose(fd);
      return "";
    }

    char buf[256];
    memset(buf, 0, sizeof(buf));

    while (fgets(buf, sizeof(buf), fd) != nullptr) {
      this->_pos = ftell(fd);
      ss << buf;
    }

    fclose(fd);
  }
  return ss.str();
}

lavender::logging::filesystem::Watcher::Watcher(
    std::shared_ptr<lavender::OpenSearch> search,
    const std::filesystem::path& path)
    : _search(search), _root(path), _items({}) {
  if (std::filesystem::is_regular_file(path)) {
    auto file = std::make_shared<lavender::logging::filesystem::File>(path);
    this->_items[path] = file;
  } else if (std::filesystem::is_directory(path)) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
      if (std::filesystem::is_regular_file(entry)) {
        const auto it = entry.path();
        auto file = std::make_shared<lavender::logging::filesystem::File>(it);
        this->_items[it] = file;
      }
    }
  } else {
    throw std::runtime_error(path.string() + " isn't a valid filepath");
  }

  BOOST_LOG_TRIVIAL(info) << "watching on " << path.string();
  this->_file = inotify_init();
  if (this->_file < 0) {
    throw std::runtime_error(std::strerror(errno));
  }

  this->_watcher = inotify_add_watch(this->_file, path.c_str(),
                                     IN_MODIFY | IN_CREATE | IN_DELETE);
  if (this->_watcher < 0) {
    throw std::runtime_error(std::strerror(errno));
  }
}

lavender::logging::filesystem::Watcher::~Watcher() {
  inotify_rm_watch(this->_file, this->_watcher);
  close(this->_file);
}

void lavender::logging::filesystem::Watcher::sync(
    const std::filesystem::path& file) {
  auto msg = this->_items[file]->read();
  boost::algorithm::trim(msg);
  BOOST_LOG_TRIVIAL(debug) << msg;
  if (msg.empty()) {
    return;
  }
  lavender::logging::filesystem::Message it;
  it.line = msg;
  it.file = std::filesystem::absolute(file).string();
  it.host = boost::asio::ip::host_name();
  it.created_at = std::chrono::high_resolution_clock::now();
  this->_search->index_document(it);
}

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_LEN (1024 * (EVENT_SIZE + 16))

void lavender::logging::filesystem::Watcher::watch() {
  std::lock_guard<std::mutex> lock(this->_mutex);

  char buffer[BUFFER_LEN];

  auto length = read(this->_file, buffer, BUFFER_LEN);
  if (length < 0) {
    BOOST_LOG_TRIVIAL(error)
        << "read(" << errno << "): " << std::strerror(errno);
    return;
  }
  BOOST_LOG_TRIVIAL(debug) << "receive buffer(" << length << " bytes)";

  int i = 0;

  while (i < length) {
    struct inotify_event* event = (struct inotify_event*)&buffer[i];
    BOOST_LOG_TRIVIAL(debug)
        << "get event: name(" << event->name << ") length(" << event->len
        << ") cookie(" << event->cookie << ") mask(" << event->mask << ")";

    if (event->len) {
      if (event->mask & IN_CREATE) {
        if (event->mask & IN_ISDIR) {
          BOOST_LOG_TRIVIAL(info)
              << "directory " << event->name << " was created";
        } else {
          const auto file = this->_root / event->name;
          BOOST_LOG_TRIVIAL(info) << "file " << file.string() << " was created";
          auto it = std::make_shared<lavender::logging::filesystem::File>(file);
          this->_items[file] = it;
        }
      } else if (event->mask & IN_DELETE) {
        if (event->mask & IN_ISDIR) {
          BOOST_LOG_TRIVIAL(info)
              << "directory " << event->name << " was deleted";
        } else {
          const auto file = this->_root / event->name;
          BOOST_LOG_TRIVIAL(info) << "file " << file.string() << " was deleted";
          this->_items.erase(file);
        }
      } else if (event->mask & IN_MODIFY) {
        if (event->mask & IN_ISDIR) {
          BOOST_LOG_TRIVIAL(info)
              << "directory " << event->name << " was modified";
        } else {
          const auto file = this->_root / event->name;
          BOOST_LOG_TRIVIAL(info)
              << "file " << file.string() << " was modified";
          this->sync(file);
        }
      }
    } else {
      if (event->mask & IN_MODIFY) {
        BOOST_LOG_TRIVIAL(info)
            << "file " << this->_root.string() << " was modified";
        this->sync(this->_root);
      }
    }
    i += EVENT_SIZE + event->len;
  }
}
