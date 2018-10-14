#include <cassert>
#include <string>

#include <zmq.hpp>

#include "json.hpp"

using json = nlohmann::json;
const double PI = 3.141592653589793238463;

// begin: utility functions
inline std::string toString(const zmq::message_t &reply) {
  return (std::string(static_cast<const char *>(reply.data()), reply.size()));
}

inline void printReply(const zmq::message_t &reply) {
  std::string result =
      std::string(static_cast<const char *>(reply.data()), reply.size());
  std::cout << result << "\n";
}

void print(const json &reply) {
  for (auto &element : reply) {
    std::cout << element << '\n';
  }
}
inline bool check(const json &reply) {
  return (reply["status"] == "OK" ? true : false);
}
// end: utility functions

namespace graff {

class Distribution {
public:
  virtual json ToJson(void) const {
    json j;
    return (j);
  }
};

class Normal : public Distribution {
  std::vector<double> mean_;
  std::vector<double> cov_;

public:
  // univariate
  Normal(const double &mean, const double &cov) : mean_({mean}), cov_({cov}) {}
  // multivariate
  Normal(const std::vector<double> &mean, const std::vector<double> &cov)
      : mean_(mean), cov_(cov) {
    assert(mean.size() * mean.size() == cov.size());
  }
  json ToJson(void) const {
    json j;
    j["type"] = "Normal";
    j["mean"] = mean_;
    j["cov"] = cov_;
    return (j);
  }
};

class SampleWeights : public Distribution {
  std::vector<double> samples_;
  std::vector<double> weights_;

public:
  SampleWeights(const std::vector<double> &samples,
                const std::vector<double> &weights)
      : samples_(samples), weights_(weights) {}
  json ToJson(void) const {
    json j;
    j["type"] = "SampleWeights";
    j["samples"] = samples_;
    j["weights"] = weights_;
    return j;
  }
};

// base class - captures a generic entity/object
class Element {
  std::string name_;
  std::string type_;

public:
  Element(const std::string &name, const std::string &type)
      : name_(name), type_(type){};
  virtual std::string name(void) const { return (name_); }
  virtual void SetName(const std::string &name) { name_ = name; }
  virtual ~Element(){};
  virtual json ToJson(void) {
    json j;
    j["label"] = name_;
    j["type"] = type_;
    return (j);
  };
};

class Endpoint {
  zmq::context_t context_;
  zmq::socket_t socket_;

public:
  Endpoint() : context_(1), socket_(context_, ZMQ_REQ) {}

  void Connect(const std::string &address) { socket_.connect(address.c_str()); }

  void Disconnect(void) {}

  json SendRequest(const json &request) {
    std::string request_str = request.dump(0);
    zmq::message_t request_msg(request_str.length()), reply_msg;

    memcpy(request_msg.data(), request_str.c_str(), request_str.length());
    socket_.send(request_msg);

    json reply;
    if (socket_.recv(&reply_msg) < 0) {
      std::cerr << "Something went wrong: " << toString(reply_msg) << "\n";
    } else {
      reply = json::parse(toString(reply_msg));
    }
    return (reply);
  };

  json Status(void) {
    json request;
    request["type"] = "getStatus";
    return (SendRequest(request));
  }
};

class Variable : public Element {
  // not really much in here for now...
public:
  Variable(const std::string &name, const std::string &type)
      : Element(name, type) {}
};

// key class
class Factor : public Element {
  std::string type_;
  std::vector<std::string> variables_;
  // a factor can take either a single distribution or one distribution per
  // measurement axis (e.g. priorpoint2 is a 2dof normal, but a RAE comprises 3
  // distributions )
  std::vector<graff::Distribution> distributions_;

  static std::string cat(const std::vector<std::string> &v) {
    // this creates a factor label according to the caesar convention
    // concatenates the variable names, prepending them with an 'f'
    std::string c("f");
    for (unsigned int i = 0; i < v.size(); ++i) {
      c += v[i];
    }
    return (c);
  }

public:
  // single variable, single measurement distribution
  Factor(const std::string &type, const std::string variable,
         const Distribution &distribution)
      : Element(std::string("f" + variable), type), variables_({variable}),
        distributions_({distribution}) {}
  // single variable, multiple measurement distributions
  Factor(const std::string &type, const std::string variable,
         const std::vector<Distribution> &distribution)
      : Element(std::string("f" + variable), type), variables_({variable}),
        distributions_(distribution) {}
  // multiple variables, single measurement distribution
  Factor(const std::string &type, const std::vector<std::string> variables,
         const Distribution &distribution)
      : Element(cat(variables), type), variables_(variables),
        distributions_({distribution}) {}
  // multiple variables, multiple measurement distributions
  Factor(const std::string &type, const std::vector<std::string> variables,
         const std::vector<Distribution> &distribution)
      : Element(cat(variables), type), variables_(variables),
        distributions_(distribution) {}

  virtual json ToJson(void) {
    json j;
    j["label"] = name();
    j["variables"] = variables_; // the variable labels
    if (distributions_.size() > 1) {
      for (unsigned int i = 0; i < 0; ++i) {
        j["measurement"][std::to_string(i)] = distributions_[i].ToJson();
      }
    } else {
      j["measurement"] = distributions_[0].ToJson();
    }
    return (j);
  }
};

class Robot {
  std::string name_;

public:
  Robot() {}
  Robot(const std::string &name) : name_(name) {}
  std::string name(void) const { return (name_); }
};

class Session {
  std::string name_;
  std::vector<graff::Variable> variables_;
  std::vector<graff::Factor> factors_;
  /*
   may want to replace variables_ with an std::map to facilitate querying, for
   instance: Distribution GetKDEMax(endpoint, session, "x0");
  */

public:
  Session() {}
  Session(const std::string &name) : name_(name) {}
  void AddVariable(const graff::Variable &variable) {
    variables_.push_back(variable);
  };
  void AddFactor(const graff::Factor &factor) { factors_.push_back(factor); };
  std::string name(void) const { return (name_); }

  json ToJson(void) {
    json j;
    j["name"] = name_;
    for (unsigned int i = 0; i < variables_.size(); ++i) {
      j["variables"][variables_[i].name()] = variables_[i].ToJson();
    }
    for (unsigned int i = 0; i < factors_.size(); ++i) {
      j["factors"][factors_[i].name()] = factors_[i].ToJson();
    }
    return (j);
  }
};
} // namespace graff

json AddVariable(graff::Endpoint &ep, graff::Session s, graff::Variable v) {
  json request, reply;
  request["type"] = "addVariable";
  request["variable"] = v.ToJson();
  reply = ep.SendRequest(request);
  if (check(reply)) {
    s.AddVariable(v);
  } else {
    std::cerr << "Request failed!" << std::endl;
    std::cerr << "Request contents:\n";
    std::cerr << request;
    std::cerr << "\n\n\n" << std::endl;
  }

  return (reply);
}

json AddFactor(graff::Endpoint &ep, graff::Session s, graff::Factor f) {
  json request, reply;
  request["type"] = "addFactor";
  request["factor"] = f.ToJson();
  reply = ep.SendRequest(request);
  if (check(reply)) {
    s.AddFactor(f);
  } else {
    std::cerr << "Request failed!" << std::endl;
  }
  return (reply);
}

// TODO: revise to comply with GSG (arguments are value, const ref, or
// pointers!)
json RegisterRobot(graff::Endpoint &ep, graff::Robot robot) {
  json request, reply;
  request["type"] = "registerRobot";
  request["robot"] = robot.name();
  return (ep.SendRequest(request));
}

json RegisterSession(graff::Endpoint &ep, graff::Robot robot,
                     graff::Session session) {
  json request, reply;
  request["type"] = "registerSession";
  request["robot"] = robot.name();
  request["session"] = session.name();
  return (ep.SendRequest(request));
}

// update the local estimates
json UpdateSession(graff::Endpoint &ep, graff::Session &s) {
  json reply;
  // TODO: implementation
  return (reply);
}

json RequestSolve(graff::Endpoint &ep, graff::Session &s) {
  json request;
  request["type"] = "batchSolve";
  return (ep.SendRequest(request));
}

json GetVarMAPKDE(graff::Endpoint &ep, graff::Session &s,
                   const std::string &variable) {
  json request;
  request["type"] = "GetVarMAPKDE";
  request["variable"] = variable;
  return (ep.SendRequest(request));
}

json GetVarMAPMax(graff::Endpoint &ep, graff::Session &s,
                   const std::string &variable) {
  json request;
  request["type"] = "GetVarMAPMax";
  request["variable"] = variable;
  return (ep.SendRequest(request));
}

json GetVarMAPMean(graff::Endpoint &ep, graff::Session &s,
                    const std::string &variable) {
  json request;
  request["type"] = "GetVarMAPMean";
  request["variable"] = variable;
  return (ep.SendRequest(request));
}

json RequestShutdown(graff::Endpoint &ep) {
  json request;
  request["type"] = "shutdown";
  return (ep.SendRequest(request));
}

json ToggleMockMode(graff::Endpoint &ep) {
  json request;
  request["type"] = "toggleMockServer";
  return (ep.SendRequest(request));
}

json GetVarsByTag(graff::Endpoint &ep, const std::string &tag) {
  json request;
  request["type"] = "varQuery";
  request["tag"] = tag;
  return (ep.SendRequest(request));
}
