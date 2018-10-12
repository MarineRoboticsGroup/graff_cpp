#include <map>
#include <string>

#include <zmq.hpp>

#include "json.hpp"

// TODO: matrices become column-major vectors
// TODO: avoid two-stage packing on distributions

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
  // potentially useful reference:
  // https://juliastats.github.io/Distributions.jl/latest/univariate.html#Common-Interface-1
public:
  virtual json ToJson(void) const {
    json j;
    return (j);
  }
};

class MvNormal : public Distribution {
  std::vector<double> mean_;
  std::vector<std::vector<double>> cov_;

public:
  MvNormal(const std::vector<double> &mean,
           const std::vector<std::vector<double>> &cov)
      : mean_(mean), cov_(cov) {}
  // TODO: object from JSON
  json ToJson(void) const {
    json j;
    j["type"] = "MvNormal";
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
  // TODO: object from JSON
  // SampleWeights(const json &dict)
  //   : samples_(dict["samples"]), weights_(dict["weights"]) {}
  json ToJson(void) const {
    json j;
    j["type"] = "SampleWeights";
    j["samples"] = samples_;
    j["weights"] = weights_;
    return j;
  }
};

// base class
class Element {
  std::string name_;

public:
  Element(const std::string &name) : name_(name){};
  virtual std::string name(void) const { return (name_); }
  virtual void SetName(const std::string &name) { name_ = name; }
  virtual ~Element(){};
  virtual json ToJson(void) {
    json j;
    j["name"] = name_;
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
public:
  Variable() : Element("x0") {}
  Variable(const std::string &name) : Element(name) {}
};

class Point2 : public Variable {
public:
  Point2(const std::string &name) : Variable(name) {}
  json ToJson(void) {
    json j;
    j["name"] = name();
    j["type"] = "Point2";
    return (j);
  }
};

class Pose2 : public Variable {
public:
  Pose2() : Variable("x0") {}
  Pose2(const std::string &name) {}
  json ToJson(void) {
    json j;
    j["name"] = name();
    j["type"] = "Pose2";
    return (j);
  }
};

class Point3 : public Variable {
public:
  Point3(const std::string &name) : Variable(name) {}
  json ToJson(void) {
    json j;
    j["name"] = name();
    j["type"] = "Point3";
    return (j);
  }
};

class Pose3 : public Variable {
public:
  Pose3() : Variable("x0") {}
  Pose3(const std::string &name) {}
  json ToJson(void) {
    json j;
    j["name"] = name();
    j["type"] = "Pose3";
    return (j);
  }
};

// key class
class Factor : public Element {
  std::string type_;
  std::vector<std::string> variables_;
  graff::Distribution distribution_;

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
  // TODO: empty
  // Factor() : Element("fx0") {}
  // Factor(const std::string &name) : Element(name) {}
  // single variable
  // Factor(const std::string &type, const std::string variable,
  //        const Distribution &distribution)
  //     : Element(ame), distribution_(distribution) {
  //   variables_.push_back(variable);
  // }
  Factor(const std::string &type, const std::string variable,
         const Distribution &distribution)
      : Element(std::string("f" + variable)), type_(type),
        distribution_(distribution) {
    variables_.push_back(variable);
  }
  Factor(const std::string &type, const std::vector<std::string> variables,
         const Distribution &distribution)
      : Element(cat(variables)), type_(type), variables_(variables),
        distribution_(distribution) {}

  virtual json ToJson(void) {
    json j;
    j["name"] = name();
    j["variables"] = variables_; // the variable labels
    j["measurement"] = distribution_.ToJson();
    return (j);
  }

  std::vector<std::string> Variables(void) const { return (variables_); }
  graff::Distribution Distribution(void) const { return (distribution_); }
};

/*
class PriorPose3 : public Factor {
public:
PriorPose3(const std::string &variable, const graff::Distribution &distribution)
    : Factor(variable, distribution) {}
json ToJson(void) {
  json j;
  j["name"] = name();
  j["variables"] = Variables(); // the variable labels
  j["measurement"] = Distribution().ToJson();
  j["type"] = "PriorPose3";
  return (j);
}
};
*/

/*
class Odometry2 : public Factor {
graff::Distribution meas_;

public:
Odometry2(const std::vector<std::string> &variables,
          const graff::Distribution &meas)
    : Factor(cat(variables), variables), meas_(meas) {}
};
*/

class Robot : public Element {
public:
  Robot() : Element("robot") {}
  Robot(const std::string &name) : Element(name) {}
};

class Session : public Element {
  // TODO: replace with
  std::map<std::string, graff::Variable> variables2_;
  std::vector<graff::Variable> variables_;
  std::vector<graff::Factor> factors_;

public:
  Session() : Element("session") {}
  Session(const std::string &name) : Element(name) {}
  void AddVariable(const graff::Variable &variable) {
    variables_.push_back(variable);
  };
  void AddFactor(const graff::Factor &factor) { factors_.push_back(factor); };
};
} // namespace graff

json AddVariable(graff::Endpoint &ep, graff::Session s, graff::Variable v) {
  json request, reply;
  request["type"] = "addVariable";
  request["variable"] = v.ToJson();
  reply = ep.SendRequest(request);
  if (check(reply))
    s.AddVariable(v);
  return (reply);
}

json AddFactor(graff::Endpoint &ep, graff::Session s, graff::Factor f) {
  json request, reply;
  request["type"] = "addFactor";
  request["factor"] = f.ToJson();
  reply = ep.SendRequest(request);
  if (check(reply))
    s.AddFactor(f);
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
  return (reply);
}
