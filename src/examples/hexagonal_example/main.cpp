#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <zmq.hpp>

#include "json.hpp"
using json = nlohmann::json;

const double PI = 3.141592653589793238463;

inline std::string toString(const zmq::message_t &reply) {
  return (std::string(static_cast<const char *>(reply.data()), reply.size()));
}

// Serialize and send a json request to the socket, then wait for a reply.
inline int sendRequest(const json &request, zmq::socket_t &socket,
                       zmq::message_t &reply) {
  std::string request_str = request.dump(2); // serialize
  zmq::message_t msg(request_str.length());
  memcpy(msg.data(), request_str.c_str(), request_str.length());
  socket.send(msg);

  if (socket.recv(&reply) < 0) {
    std::cerr << "Something went wrong: " << toString(reply) << "\n";
    return (-1);
  }
  return (0);
}

inline void printReply(const zmq::message_t &reply) {
  std::string result =
      std::string(static_cast<const char *>(reply.data()), reply.size());
  std::cout << result << "\n";
}

int getStatus(zmq::socket_t &socket) {
  // create request
  json request;
  request["type"] = "GetStatus";

  // send request
  zmq::message_t reply;
  if (sendRequest(request, socket, reply)) {
    return (-1);
  }

  // parse reply
  std::string r = toString(reply);
  if (r.compare("OK")) {
    std::cerr << "Server is unhappy with request.\n";
    return (-1);
  }
  return (0);
}

int registerRobot(zmq::socket_t &socket, const std::string &id,
                  const std::string &name = "robot",
                  const std::string &description = "my robot",
                  const std::string &status = "aok") {
  json request;
  request["type"] = "RegisterRobot";
  request["id"] = id;
  request["name"] = name;
  request["description"] = description;
  request["status"] = status;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here

  return (0);
}

int registerSession(zmq::socket_t &socket, const std::string &session_id) {
  json request;
  request["type"] = "RegisterSession";
  request["session_id"] = session_id;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here

  return (0);
}

int addOdometry2D(zmq::socket_t &socket, const std::string &robot_id,
                  const std::string &session_id,
                  const std::vector<double> &measurement,
                  const std::vector<std::vector<double>> &covariance) {
  json request;
  request["robot_id"] = robot_id;
  request["session_id"] = session_id;
  request["type"] = "AddOdometry2D";
  request["measurement"] = measurement;
  request["covariance"] = covariance;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here

  return (0);
}

int addLandmark2D(zmq::socket_t &socket, const std::string &robot_id,
                  const std::string &session_id,
                  const std::string &landmark_id) {
  json request;
  request["type"] = "addLandmark2D";
  request["robot_id"] = robot_id;
  request["session_id"] = session_id;
  request["landmark_id"] = landmark_id;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here

  return (0);
}
int addFactor_BearingRangeNormal(zmq::socket_t &socket,
                                 const std::string &robot_id,
                                 const std::string &session_id,
                                 const std::string &pose_id,
                                 const std::string &landmark_id,
                                 const double &bearing, const double &range) {
  json request;
  request["type"] = "addFactor_BearingRangeNormal";
  request["robot_id"] = robot_id;
  request["session_id"] = session_id;
  request["pose_id"] = pose_id;
  request["landmark_id"] = landmark_id;
  request["bearing"] = bearing;
  request["range"] = range;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here

  return (0);
}

int setReady(zmq::socket_t &socket, const std::string &robot_id,
             const std::string &session_id, const bool &ready) {
  json request;
  request["type"] = "setReady";
  request["robot_id"] = robot_id;
  request["session_id"] = session_id;
  request["ready"] = ready;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here

  return (0);
}

int main(int argCount, char **argValues) {
  //  Prepare our context and socket

  zmq::context_t context(1);
  zmq::socket_t socket(context, ZMQ_REQ);

  std::cout << "Connecting to navi server…" << std::endl;
  socket.connect("tcp://localhost:5555");

  std::string robot_id = "Hexagonal";
  std::string session_id = "cjz002";

  if (registerRobot(socket, robot_id)) {
    std::cerr << "Failed to register robot!\n";
    return (-1);
  }

  if (registerSession(socket, session_id)) {
    std::cerr << "Failed to register session!\n";
    return (-1);
  }

  // create odometry chain
  for (int i = 0; i < 6; ++i) {

    std::vector<double> measurement = {10.0, 0.0, PI / 3.0};
    std::vector<std::vector<double>> covariance = {
        {0.1, 0.0, 0.1}, {0.1, 0.0, 0.1}, {0.1, 0.0, 0.1}};
    addOdometry2D(socket, robot_id, session_id, measurement, covariance);

    // TODO: addOrUpdateDataElement
  }

  // add landmark
  addLandmark2D(socket, robot_id, session_id, "l1");
  // add loop closures between {x1, x6} and l1
  addFactor_BearingRangeNormal(socket, robot_id, session_id, "x1", "l1", 0.0,
                               20.0);
  addFactor_BearingRangeNormal(socket, robot_id, session_id, "x6", "l1", 0.0,
                               20.0);

  // flag the session as ready for a new
  // solution
  setReady(socket, robot_id, session_id, true);

  // get the session

  return (0);
}
