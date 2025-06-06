#pragma once
#include "GParsing/GParsing.hpp"

namespace Wepp {
bool HandleWeb(GParsing::HTTPRequest _req,
                           GParsing::HTTPResponse &_resp,
                           bool &_closeConnection);

bool HandleWebPost(GParsing::HTTPRequest _req, GParsing::HTTPResponse &_resp);
} // namespace Wepp
