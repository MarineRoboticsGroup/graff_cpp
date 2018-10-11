#include <map>
#include <string>

#include <zmq.hpp>

#include "json.hpp"
using json = nlohmann::json;

const double PI = 3.141592653589793238463;

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
  return (reply["status"] == "ok" ? true : false);
}

namespace graff {

class Distribution {
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
  json ToJson(void) const {
    json j;
    j["mean"] = mean_;
    j["cov"] = cov_;
    return (j);
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

  // Endpoint(const graff::Endpoint& ep): context_(ep.context_),
  // socket_(ep.socket_) {}

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

class Node : public Element {
public:
  Node() : Element("x0") {}
  Node(const std::string &name) : Element(name) {}
};

class Point2 : public Node {
  double x_, y_;
};

class Pose2 : public Node {
  double x_, y_, h_;

public:
  Pose2() : Node("x0"), x_(0), y_(0), h_(0) {}
  Pose2(const std::string &name) : Node(name), x_(0), y_(0), h_(0) {}
  Pose2(const std::string &name, const double &x, const double &y,
        const double &h)
      : Node(name), x_(x), y_(y), h_(h){};
  json ToJson(void) {
    json j;
    j["name"] = name();
    j["type"] = "Pose2";
    return (j);
  }
};

class Point3 : public Node {
  double x_, y_, z_;
};

class Pose3 : public Node {
  double x_, y_, z_, qw_, qx_, qy_, qz_;
}

class Factor : public Element {
  graff::Distribution d;
  std::vector<std::string> nodes_;

public:
  Factor() : Element("fx0") {}
  Factor(const std::string &name) : Element(name) {}
  Factor(const std::string &name, const std::vector<std::string> nodes)
      : Element(name), nodes_(nodes) {}

  json ToJson(void) {
    json j;
    j["name"] = name();
    j["nodes"] = nodes_; // the node labels
    j["measurement"] = d.ToJson();
    return (j);
  }
};

class PriorPose2 : public Factor {
  graff::Distribution meas_;
};

class Odometry2 : public Factor {
  graff::Distribution meas_;

  static std::string cat(const std::vector<std::string> &v) {
    std::string c("f");
    for (unsigned int i = 0; i < v.size(); ++i) {
      c += v[i];
    }
    return (c);
  }

public:
  Odometry2(const std::vector<std::string> &nodes,
            const graff::Distribution &meas)
      : Factor(cat(nodes), nodes), meas_(meas) {}
};

class Robot : public Element {
public:
  Robot() : Element("robot") {}
  Robot(const std::string &name) : Element(name) {}
};

class Session : public Element {
  // TODO: replace with
  std::map<std::string, graff::Node> nodes2_;
  std::vector<graff::Node> nodes_;
  std::vector<graff::Factor> factors_;

public:
  Session() : Element("session") {}
  Session(const std::string &name) : Element(name) {}
  void AddNode(const graff::Node &node) { nodes_.push_back(node); };
  void AddFactor(const graff::Factor &factor) { factors_.push_back(factor); };
};
} // namespace graff

json AddNode(graff::Endpoint &ep, graff::Session s, graff::Node n) {
  json request, reply;
  request["type"] = "addNode";
  request["node"] = n.ToJson();
  reply = ep.SendRequest(request);
  if (check(reply))
    s.AddNode(n);
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
