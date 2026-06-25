#pragma once

#include <format>

#include <cpr/cpr.h>
#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <boost/type_index.hpp>
#include <nlohmann/json.hpp>

namespace lavender {

namespace http {
namespace headers {
inline static const std::string CONTENT_TYPE = "Content-Type";
}
namespace content_types {
inline static const std::string APPLICATION_JSON_UTF8 =
    "application/json; charset=utf-8";
}
}  // namespace http

class OpenSearch {
 public:
  OpenSearch(const std::string& url, const std::string& namespace_)
      : _url(url), _namespace(namespace_) {
    BOOST_LOG_TRIVIAL(debug) << "open " << this->_url;
  }

  template <typename T>
  bool index_document(const T& object) const {
    const auto name = this->index<T>();
    BOOST_LOG_TRIVIAL(debug) << "index document " << name;
    nlohmann::json body;
    nlohmann::to_json(body, object);

    // BOOST_LOG_TRIVIAL(debug) << body.dump();

    cpr::Response res = cpr::Post(
        cpr::Url{this->url(name) + "/_doc/"}, cpr::Body{body.dump()},
        cpr::Header{{lavender::http::headers::CONTENT_TYPE,
                     lavender::http::content_types::APPLICATION_JSON_UTF8}});
    BOOST_LOG_TRIVIAL(debug) << res.status_code << " " << res.text;
    return res.status_code == 200;
  }

  template <typename T>
  bool index_document(const std::string& id, const T& object) const {
    const auto name = this->index<T>();
    BOOST_LOG_TRIVIAL(debug) << "index document " << id << " " << name;
    nlohmann::json body;
    nlohmann::to_json(body, object);

    // BOOST_LOG_TRIVIAL(debug) << body.dump();

    cpr::Response res = cpr::Put(
        cpr::Url{this->url(name) + "/_doc/" + id}, cpr::Body{body.dump()},
        cpr::Header{{lavender::http::headers::CONTENT_TYPE,
                     lavender::http::content_types::APPLICATION_JSON_UTF8}});
    BOOST_LOG_TRIVIAL(debug) << res.status_code << " " << res.text;
    return res.status_code == 200;
  }

  template <typename T>
  bool index_exists() const {
    const auto name = this->index<T>();
    BOOST_LOG_TRIVIAL(debug) << "index exists " << name;
    cpr::Response res = cpr::Head(cpr::Url{this->url(name)});
    BOOST_LOG_TRIVIAL(debug) << res.status_code << " " << res.text;
    return res.status_code == 200;
  }

  // https://docs.opensearch.org/latest/mappings/supported-field-types/index/
  template <typename T>
  bool create_index(const nlohmann::json& properties) const {
    const auto name = this->index<T>();
    nlohmann::json mappings;
    mappings["properties"] = properties;
    nlohmann::json config;
    config["mappings"] = mappings;

    const std::string body = config.dump();
    BOOST_LOG_TRIVIAL(warning) << "create index " << name << ": " << body;
    cpr::Response res = cpr::Put(
        cpr::Url{this->url(name)}, cpr::Body{body},
        cpr::Header{{lavender::http::headers::CONTENT_TYPE,
                     lavender::http::content_types::APPLICATION_JSON_UTF8}});
    if (res.status_code != 200) {
      BOOST_LOG_TRIVIAL(error) << res.status_code << " " << res.text;
      return false;
    }
    BOOST_LOG_TRIVIAL(info) << res.text;

    return true;
  }

  inline std::string url(const std::string& path) const {
    const std::string it = std::format("{}/{}", this->_url, path);
    return it;
  }

 private:
  //  https://docs.opensearch.org/latest/api-reference/index-apis/create-index/#index-naming-restrictions
  template <typename T>
  std::string index() const {
    const std::string ty = boost::typeindex::type_id<T>().pretty_name();
    std::string it = std::format("{}-{}", this->_namespace, ty);
    boost::algorithm::replace_all(it, "::", ".");
    boost::algorithm::to_lower(it);
    return it;
  }

  std::string _url;
  std::string _namespace;
};
}  // namespace lavender
