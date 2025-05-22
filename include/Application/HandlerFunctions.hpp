#pragma once
#include "GParsing/GParsing.hpp"

namespace Wepp {
bool HandleRequestResponse(GParsing::HTTPRequest _req,
                           GParsing::HTTPResponse &_resp,
                           bool &_closeConnection);

bool HandlerContinue(GParsing::HTTPRequest _req, GParsing::HTTPResponse &_resp);
} // namespace Wepp
