// GraffSDK

#include <iostream>
#include "json.hpp"


struct GraffConfig {
  std::string userId;
  std::string robotId;
  std::string sessionId;
};

std::string getStatus(const GraffConfig &cfg);
