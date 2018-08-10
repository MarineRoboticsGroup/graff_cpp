// GraffSDK
#include "GraffSDK.h"
#include <iostream>
#include "json.hpp"

using nlohmann::json;


std::string getStatus(const GraffConfig &cfg) {
  json request;
  request["userId"] = cfg.userId;
  request["robotId"] = cfg.robotId;
  request["sessionId"] = cfg.sessionId;
  request["type"] = "getStatus";
  std::string request_str = request.dump(2); // serialize
  return request_str;
}
