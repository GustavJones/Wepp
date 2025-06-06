#include "Application/HandlerFunctions.hpp"
#include "Application/HTTPChecks.hpp"
#include "FileHandling/FileIO.hpp"
#include "GLog/Log.hpp"
#include <filesystem>

namespace Wepp {
bool HandleWeb(GParsing::HTTPRequest _req,
                           GParsing::HTTPResponse &_resp,
                           bool &_closeConnection) {
  std::vector<unsigned char> buffer;

  if (!HasHostHeader(_req)) {
    GLog::Log(GLog::LOG_WARNING,
              "[Handler]: Connection request did not provide a Host header");
    _resp.version = "HTTP/1.1";
    _resp.response_code = 400;
    _resp.response_code_message = "Bad Request";
    _resp.headers.push_back({"Connection", {"close"}});
    return false;
  }

  if (_req.uri.find('/') == _req.uri.find("//")) {
    int index = _req.uri.find("//") + 2;
    _req.uri.erase(0, index);
  }

  _req.uri.erase(0, _req.uri.find('/') + 1);

  if (_req.uri == "") {
    _req.uri = "index.html";
  }

  _resp.version = "HTTP/1.1";
  _resp.response_code = 200;
  _resp.response_code_message = "OK";

  GLog::Log(GLog::LOG_TRACE, "[Handler]: Requesting URI - " +
                                 std::filesystem::absolute(_req.uri).string());
  if (std::filesystem::exists(std::filesystem::absolute(
          (std::filesystem::path) "data" / _req.uri))) {
    GLog::Log(GLog::LOG_TRACE, "[Handler]: File found!");
    buffer.resize(Wepp::FileSize(
        std::filesystem::absolute((std::filesystem::path) "data" / _req.uri)));
    Wepp::ReadFile(
        std::filesystem::absolute((std::filesystem::path) "data" / _req.uri),
        buffer);

    _resp.message = buffer;
  } else {
    GLog::Log(GLog::LOG_TRACE, "[Handler]: File NOT found!");
    _resp.response_code = 404;
    _resp.response_code_message = "Not Found";
  }

  _resp.headers.push_back({"Connection", {"close"}});
  _closeConnection = true;

  return true;
}

bool HandleWebPost(GParsing::HTTPRequest _req,
                     GParsing::HTTPResponse &_resp) {
  _resp.version = "HTTP/1.1";
  _resp.response_code = 100;
  _resp.response_code_message = "Continue";
  _resp.headers.push_back({"Connection", {"keep-alive"}});
  return true;
}
} // namespace Wepp
