#pragma once
#include "GParsing/GParsing.hpp"

namespace Wepp {
bool HandleApi(GParsing::HTTPRequest _req,
                           GParsing::HTTPResponse &_resp,
                           bool &_closeConnection);

bool HandleApiPost(GParsing::HTTPRequest _req, GParsing::HTTPResponse &_resp);
} // namespace Wepp
