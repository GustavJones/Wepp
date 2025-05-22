#include "Api/HandlerFunctions.hpp"

namespace Wepp {
bool HandleApi(GParsing::HTTPRequest _req, GParsing::HTTPResponse &_resp,
               bool &_closeConnection) {
  return true;
}

bool HandleApiPost(GParsing::HTTPRequest _req, GParsing::HTTPResponse &_resp) {
  return false;
}
} // namespace Wepp
