#include <cassert>
#include <string>

#include <zmq.hpp>

#include "json.hpp"

using json = nlohmann::json;
const double PI = 3.141592653589793238463;

// BEGIN: utility functions
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

/*! \class Distribution graff.hpp
 *  \brief A class to model a generic distribution object.
 *
 * See also: visitor pattern
 *
 */
class Distribution {
public:
  virtual json ToJson(void) const = 0;
};

/*!
 * \class Normal graff.hpp
 * \brief A class to handle a uni- or multi-variate normal distribution.
 */
class Normal : public Distribution {
  std::vector<double> mean_; /*!< mean vector */
  std::vector<double> cov_;  /*!< covariance matrix, in column-major order */

public:
  /*!
   * \brief Constructor for a univariate normal
   * \param [in] mean Mean
   * \param [in] var Variance
   */
  Normal(const double &mean, const double &var) : mean_({mean}), cov_({var}) {}

  /*!
   * \brief Constructor for a multivariate normal
   * \param [in] mean Mean vector
   * \param [in] var Covariance matrix, in column-major order.
   */
  Normal(const std::vector<double> &mean, const std::vector<double> &cov)
      : mean_(mean), cov_(cov) {
    assert(mean.size() * mean.size() == cov.size());
  }
  /*! \brief Encode the distribution as a JSON object.
   *  \return The JSON-encoded distribution object.
   */
  json ToJson(void) const override {
    json j;
    j["distType"] = "MvNormal";
    j["mean"] = mean_;
    j["cov"] = cov_;
    return (j);
  }
};

/*!
 * \class SampleWeights
 * \brief An empirical univariate distribution defined by a set of samples and
 * associated weights.
 */
class SampleWeights : public Distribution {
  std::vector<double> samples_;
  std::vector<double> weights_;
  double quantile_;

public:
  /*! \brief
   * \param [in] samples  A vector of samples
   * \param [in] weights  A vector of weights (associated 1-to-1 with the
   * samples)
   * \param [in] quantile A value specifying the lower quantile of
   * samples to be discarded (setting this to zero forces all samples to be
   * considered).
   */
  SampleWeights(const std::vector<double> &samples,
                const std::vector<double> &weights, const double &quantile)
      : samples_(samples), weights_(weights), quantile_(quantile) {}

  /*! \brief Encode the distribution as a JSON object.
   *  \return The JSON-encoded distribution object.
   */
  json ToJson(void) const override {
    json j;
    j["distType"] = "SampleWeights"; // maybe AliasingScalarSampler?
    j["samples"] = samples_;
    j["weights"] = weights_;
    j["quantile"] = quantile_;
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
  virtual std::string Type(void) const { return (type_); }
  virtual void SetName(const std::string &name) { name_ = name; }
  virtual ~Element(){};
  virtual json ToJson(void) const {
    json j;
    j["label"] = name_;
    j["type"] = type_;
    return (j);
  };
};

/*!
 * @class Endpoint
 * @brief The main class to handle connections to the Caesar endpoint.
 */
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
    if (!socket_.recv(&reply_msg)) {
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

/*!
 * @class
 * @brief
 */
class Variable : public Element {
  // not really much in here for now...
public:
  Variable(const std::string &name, const std::string &type)
      : Element(name, type) {}
  json ToJson(void) const {
    json j;
    j["label"] = name();
    j["variableType"] = Type();
    j["N"] = 0;
    j["labels"] = {""};
    return (j);
  }
};

/*!
 * \class
 * \brief
 */
class Factor : public Element {
  // std::string type_;
  std::vector<std::string> variables_;
  // a factor can take either a single distribution or one distribution per
  // measurement axis (e.g. priorpoint2 is a 2dof normal, but a RAE comprises 3
  // distributions )
  std::vector<graff::Distribution *> distribution_ptrs_;

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
  Factor(const std::string &type) : Element("", type) {}
  Factor(const std::string &type, const std::string &variable)
      : Element("", type), variables_({variable}) {}
  Factor(const std::string &type, const std::vector<std::string> &variables)
      : Element("", type), variables_(variables) {}
  /*
  Factor(const std::string &type, const std::string variable,
         Distribution *distribution_ptr)
      : Element(std::string("f" + variable), type), variables_({variable}),
        distribution_ptrs_({distribution_ptr}) {}
  // single variable, multiple measurement distributions
  Factor(const std::string &type, const std::string variable,
         std::vector<Distribution *> distribution_ptrs)
      : Element(std::string("f" + variable), type), variables_({variable}),
        distribution_ptrs_(distribution_ptrs) {}
  // multiple variables, single measurement distribution
  Factor(const std::string &type, const std::vector<std::string> variables,
         Distribution *&distribution_ptr)
      : Element(cat(variables), type), variables_(variables),
        distribution_ptrs_({distribution_ptr}) {}
  // multiple variables, multiple measurement distributions
  Factor(const std::string &type, const std::vector<std::string> variables,
         std::vector<Distribution *> &distribution_ptrs)
      : Element(cat(variables), type), variables_(variables),
        distribution_ptrs_(distribution_ptrs) {}
  */

  void push_back(const std::string &variable) {
    variables_.push_back(variable);
  }

  void push_back(Distribution *distribution) {
    distribution_ptrs_.push_back(distribution);
  }

  void push_back(std::vector<Distribution *> distributions) {
    for (Distribution *ptr : distributions) {
      distribution_ptrs_.push_back(ptr);
    }
  }

  virtual json ToJson(void) const {
    json j;
    j["factorType"] = Type();
    // j["label"] = name(); // unneeded, as we get the label from the backend
    j["variables"] = variables_; // the variable labels
    for (unsigned int i = 0; i < distribution_ptrs_.size(); ++i) {
      j["factor"]["measurement"].emplace_back(distribution_ptrs_[i]->ToJson());
    }
    return (j);
  }
};

/*!
 * @class
 * @brief
 */
class Robot {
  std::string name_;
  std::string description_;

public:
  Robot() {}
  Robot(const std::string &name) : name_(name) {}
  Robot(const std::string &name, const std::string &description)
      : name_(name), description_(description) {}
  std::string Name(void) const { return (name_); }
  std::string Description(void) const { return (description_); }
  json ToJson(void) const {
    json j;
    j["name"] = name_;
    j["description"] = description_;
    return (j);
  }
};

/*!
 * \class
 * \brief
 */
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

/**
 * \brief Add a variable to the current session's factor graph.
 *
 * \param [in] ep The endpoint object.
 * \param [in] s The session object
 * \param [in] v The variable object.
 * \return The endpoint reply as a json object.
 */
json AddVariable(Endpoint &ep, Session s, Variable v) {
  json request, reply;
  request["request"] = "addVariable";
  request["payload"] = v.ToJson();
  reply = ep.SendRequest(request);
  if (check(reply)) {
    s.AddVariable(v);
  } else {
    std::cerr << "Request failed!" << std::endl;
    std::cerr << "Request contents:\n";
    std::cerr << request;
    std::cerr << "\n\n\n" << std::endl;
    std::cerr << "Reply contents:\n";
    std::cerr << reply;
    std::cerr << "\n\n\n" << std::endl;
  }

  return (reply);
}

/**
 * \brief Add a factor to the current session's factor graph.
 *
 * \param [in] ep The endpoint object.
 * \param [in] s The session object
 * \param [in] f The factor object.
 * \return The endpoint reply as a json object.
 */
json AddFactor(Endpoint &ep, Session s, Factor f) {
  json request, reply;
  request["request"] = "addFactor";
  request["payload"] = f.ToJson();
  // request["factor"]["factorType"] will contain the actual factor type
  reply = ep.SendRequest(request);
  if (check(reply)) {
    s.AddFactor(f);
  } else {
    std::cerr << "Request failed!" << std::endl;
    std::cerr << "Request contents:\n";
    std::cerr << request;
    std::cerr << "\n\n" << std::endl;
    std::cerr << "Reply contents:\n";
    std::cerr << reply;
    std::cerr << "\n\n" << std::endl;
  }
  return (reply);
}

// TODO: revise to comply with GSG (arguments are value, const ref, or
// pointers!)
json RegisterRobot(Endpoint &ep, Robot robot) {
  json request, reply;
  request["request"] = "registerRobot";
  request["payload"]["robot"] = robot.Name();
  return (ep.SendRequest(request));
}

json RegisterSession(Endpoint &ep, Robot robot, Session session) {
  json request, reply;
  request["request"] = "registerSession";
  request["payload"]["robot"] = robot.Name();
  request["payload"]["session"] = session.name();
  return (ep.SendRequest(request));
}

// update the local estimates
json UpdateSession(Endpoint &ep, Session &s) {
  json reply;
  // TODO: implementation
  return (reply);
}

json RequestSolve(Endpoint &ep, Session &s) {
  json request;
  request["request"] = "batchSolve";
  request["payload"] = "";
  return (ep.SendRequest(request));
}

json GetVarMAPKDE(Endpoint &ep, Session &s, const std::string &variable) {
  json request;
  request["request"] = "GetVarMAPKDE";
  request["payload"] = variable;
  return (ep.SendRequest(request));
}

json GetVarMAPMax(Endpoint &ep, Session &s, const std::string &variable) {
  json request;
  request["request"] = "GetVarMAPMax";
  request["payload"] = variable;
  return (ep.SendRequest(request));
}

json GetVarMAPMean(Endpoint &ep, Session &s, const std::string &variable) {
  json request;
  request["request"] = "GetVarMAPMean";
  request["payload"] = variable;
  return (ep.SendRequest(request));
}

json RequestShutdown(Endpoint &ep) {
  json request;
  request["request"] = "shutdown";
  request["payload"] = "";
  return (ep.SendRequest(request));
}

/**
 * \brief Toggle endpoint "mock mode"
 * In mock mode, the endpoint will acknowledge all requests, but instead of
fulfilling them, it will simply print their contents.
 * \param [in] ep The endpoint object
 */
json ToggleMockMode(Endpoint &ep, bool &mock = true) {
  json request;
  request["request"] = "toggleMockServer";
  request["payload"] = mock;
  return (ep.SendRequest(request));
}

/**
 * \brief Request a list of all variables in the current session with the
 * specified tag.
 * \param [in] ep The endpoint object.
 */
json GetVarsByTag(Endpoint &ep, const std::string &tag) {
  json request;
  request["request"] = "varQuery";
  request["payload"] = "tag";
  return (ep.SendRequest(request));
}

/**
 * \brief Request a list of all variables in the current session.
 * \param [in] ep The endpoint object.
 */
json ListVariables(Endpoint &ep) {
  json request;
  request["request"] = "ls";
  request["payload"] = "variables";
  return (ep.SendRequest(request));
}

/**
 * \brief Request a list of all factors in the current session.
 * \param [in] ep The endpoint object.
 */
json ListFactors(Endpoint &ep) {
  json request;
  request["request"] = "ls";
  request["payload"] = "factors";
  return (ep.SendRequest(request));
}

// TODO: plot commands/triggers

} // namespace graff
