#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <zmq.hpp>

#include "json.hpp"
using nlohmann::json;

const double PI = 3.141592653589793238463;

// Serialize and send a json request to the socket, then wait for a reply.
inline int sendRequest(const json &request, zmq::socket_t &socket,
                       zmq::message_t &reply) {
  std::string request_str = request.dump(2); // serialize
  zmq::message_t msg(request_str.length());
  memcpy(msg.data(), request_str.c_str(), request_str.length());
  socket.send(msg);

  return (socket.recv(&reply) < 0);
}

inline void printReply(const zmq::message_t &reply) {
  std::string result =
      std::string(static_cast<const char *>(reply.data()), reply.size());
  std::cout << result << "\n";
}

inline std::string toString(const zmq::message_t &reply) {
  return (std::string(static_cast<const char *>(reply.data()), reply.size()));
}

int registerRobot(zmq::socket_t &socket, const std::string &robot_id) {
  json request;
  request["type"] = "RegisterRobot";
  request["robot_id"] = robot_id;

  zmq::message_t reply;
  if (sendRequest(request, socket, reply)) {
    std::cerr << "Something went wrong!\n";
    std::cerr << toString(reply) << "\n";
    return (-1);
  }
  std::string r = toString(reply);
  if (r.compare("OK")) {
    std::cerr << "Server is unhappy with request.\n";
    return(-1);
  }
  return (0);
}

int registerSession(zmq::socket_t &socket, const std::string &session_id) {
  json request;
  request["type"] = "RegisterSession";
  request["session_id"] = session_id;

  zmq::message_t reply;
  if (sendRequest(request, socket, reply)) {
    std::cerr << "something went wrong!\n";
    return (-1);
  }
  printReply(reply);
  return (0);
}

int addOdometry2D(zmq::socket_t &socket, const std::string &session_id,
                  const std::string &robot_id,
                  const std::vector<double> &measurement,
                  const std::vector<std::vector<double>> &covariance) {
  json request;
  request["robot_id"] = robot_id;
  request["session_id"] = session_id;
  request["type"] = "AddOdometry2D";
  request["measurement"] = measurement;
  request["covariance"] = covariance;

  zmq::message_t reply;
  if (sendRequest(request, socket, reply)) {
    std::cerr << "something went wrong!\n";
    return (-1);
  }
  printReply(reply);
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
    // create request

    std::vector<double> measurement = {10.0, 0.0, PI / 3.0};
    std::vector<std::vector<double>> covariance = {
        {0.1, 0.0, 0.1}, {0.1, 0.0, 0.1}, {0.1, 0.0, 0.1}};
    addOdometry2D(socket, session_id, robot_id, measurement, covariance);

    // if (result.compare("OK")) {
    //   std::cerr << "Server is unhappy with request."
    //             << "\n";
    //   break;
    // }

    // add image data to the pose
    // TODO
  }

  // add landmark
  // TODO

  // add loop closures
  // TODO

  // flag the solver for attention
  // TODO

  // get the latest session
  // TODO

  // view results
  // TODO

  return (0);
}
