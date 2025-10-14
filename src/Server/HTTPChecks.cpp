#include "Wepp/Server/HTTPChecks.hpp"

namespace Wepp {
bool HasHostHeader(const GParsing::HTTPRequest &_req) {
  for (const auto &header : _req.headers) {
    if (header.first == "Host" || header.first == "host") {
      return true;
    }
  }

  return false;
}
} // namespace Wepp
