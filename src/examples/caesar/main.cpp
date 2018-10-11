#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <zmq.hpp>

#include <graff/graff.hpp>

/*
int registerRobot(zmq::socket_t &socket, const std::string &id,
                  const std::string &name = "robot",
                  const std::string &description = "my robot",
                  const std::string &status = "aok") {
  json request;
  request["type"] = "registerRobot";
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
  print(reply);

  return (0);
}
*/

/*
int registerSession(zmq::socket_t &socket, const std::string &session_id) {
  json request;
  request["type"] = "registerSession";
  request["session_id"] = session_id;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here
  print(reply);

  return (0);
}
*/

/*
int addOdometry2D(zmq::socket_t &socket, const std::string &robot_id,
                  const std::string &session_id,
                  const std::vector<double> &measurement,
                  const std::vector<std::vector<double>> &covariance) {
  json request;
  request["robotId"] = robot_id;
  request["sessionId"] = session_id;
  request["type"] = "addOdometry2D";
  request["measurement"] = measurement;
  request["covariance"] = covariance;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here
  print(reply);

  return (0);
}
*/

/*
int addLandmark2D(zmq::socket_t &socket, const std::string &robot_id,
                  const std::string &session_id,
                  const std::string &landmark_id) {
  json request;
  request["type"] = "addLandmark2D";
  request["robotId"] = robot_id;
  request["sessionId"] = session_id;
  request["landmarkId"] = landmark_id;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  // do something with the reply here
  print(reply);

  return (0);
}
*/

/*
int addFactor_BearingRangeNormal(zmq::socket_t &socket,
                                 const std::string &robot_id,
                                 const std::string &session_id,
                                 const std::string &pose_id,
                                 const std::string &landmark_id,
                                 const double &bearing, const double &range) {
  json request;
  request["type"] = "addFactorBearingRangeNormal";
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
  print(reply);

  return (0);
}
*/


/*
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
  print(reply);
  return (0);
}
*/

/*
void printRobotResponse(const json &response) {
  std::cout << "id: " << response["id"] << std::endl;
  std::cout << "name: " << response["name"] << std::endl;
  std::cout << "description: " << response["description"] << std::endl;
  std::cout << "status: " << response["status"] << std::endl;
  std::cout << "createdTimestamp: " << response["createdTimestamp"]
            << std::endl;
  std::cout << "lastUpdatedTimestamp: " << response["lastUpdatedTimestamp"]
            << std::endl;
  // for(auto it=response["links"].begin(), it!=response["links"].end() ++it){
  // }
  // TODO: print links (dict)
}
*/

/*
json getSession(zmq::socket_t &socket, const std::string &robot_id,
                const std::string &session_id) {
  json request;
  request["type"] = "getSession";
  request["robot_id"] = robot_id;
  request["session_id"] = session_id;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);

  print(reply);
  return (reply);
}
*/

// request["nodes"] =....
// request["type"] = ""
// request["distribution"] =
// request["distribution"] =
// request["distribution"]["mean"]
// request["distribution"]["covariance"]
// request["distribution"]["samples"]
// request["distribution"]["weights"]

/*
json getNode(zmq::socket_t &socket, const std::string &robot_id,
             const std::string &session_id, const std::string &node_id) {
  json request;
  request["type"] = "getSession";
  request["robot_id"] = robot_id;
  request["session_id"] = session_id;
  request["node_id"] = node_id;

  zmq::message_t message;
  if (sendRequest(request, socket, message)) {
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);

  print(reply);
  return (reply);
}
*/

int main(int argCount, char **argValues) {
  // create endpoint
  graff::Endpoint ep;

  std::cout << "Connecting to endpoint…" << std::endl;
  ep.Connect("tcp://128.30.31.88:5555");

  graff::Robot robot("krakenoid");
  graff::Session session("first dive");

  json reply;
  reply = RegisterRobot(ep, robot);
  reply = RegisterSession(ep, robot, session);

  // add a bunch of nodes a
  for (int i = 0; i < 6; ++i) {
    std::string name;
    name = "x"+std::to_string(i);
    graff::Pose2 n(name);

    reply = AddNode(ep, session, n);

    // TODO: add odometry factors
  }

  // add prior on first node

  // Initialize();
  // AddFactor();
  // AddNode();
  // GetNode();

  return (0);
}

/*

  std::string robot_id = "robot";
  std::string session_id = "session";

  std::cout << "Registering robot\n";
  if (registerRobot(socket, robot_id)) {
    std::cerr << "Failed to register robot!\n";
    return (-1);
  }

  std::cout << "Registering session\n";
  if (registerSession(socket, session_id)) {
    std::cerr << "Failed to register session!\n";
    return (-1);
  }


  // create odometry chain
  std::cout << "adding odometry chain\n";
  for (int i = 0; i < 6; ++i) {
    std::vector<double> measurement = {10.0, 0.0, PI / 3.0};
    std::vector<std::vector<double>> covariance = {
        {0.1, 0.0, 0.1}, {0.1, 0.0, 0.1}, {0.1, 0.0, 0.1}};
    addOdometry2D(socket, robot_id, session_id, measurement, covariance);

    // TODO: addOrUpdateDataElement
  }

  std::cout << "adding landmark\n";
  addLandmark2D(socket, robot_id, session_id, "l1");
  // add loop closures between {x1, x6} and l1
  addFactor_BearingRangeNormal(socket, robot_id, session_id, "x1", "l1", 0.0,
                               20.0);
  addFactor_BearingRangeNormal(socket, robot_id, session_id, "x6", "l1", 0.0,
                               20.0);

  // flag the session as ready for a solver
  setReady(socket, robot_id, session_id, true);

  // get the session
  json session = getSession(socket, robot_id, session_id);
  json x1 = getNode(socket, robot_id, session_id, "x1");

  return (0);
}
*/
