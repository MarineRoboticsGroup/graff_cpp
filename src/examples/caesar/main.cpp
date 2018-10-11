#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <graff/graff.hpp>

int main(int argCount, char **argValues) {
  graff::Endpoint ep;

  std::cout << "Connecting to endpoint…" << std::endl;
  ep.Connect("tcp://128.30.31.88:5555");

  graff::Robot robot("krakenoid");
  graff::Session session("first dive");

  json reply;
  reply = RegisterRobot(ep, robot);
  reply = RegisterSession(ep, robot, session);

  for (int i = 0; i < 6; ++i) {
    std::string name;
    name = "x" + std::to_string(i);
    graff::Pose2 n(name);

    reply = AddNode(ep, session, n); // TODO check if successful

    // TODO: add odometry factors
    if (i > 0) {
      std::string start, stop;
      start = "x" + std::to_string(i - 1);
      stop = "x" + std::to_string(i);
      std::vector<double> mu = {1.0, 0.0, PI / 6.0};
      std::vector<std::vector<double>> sig = {
          {0.01, 0.0, 0.0}, {0.0, 0.01, 0.0}, {0.0, 0.0, 0.01}};
      graff::MvNormal o(mu, sig);
      std::vector<std::string> nodes = {start,stop};
      graff::Odometry2 f(nodes, o);

      reply = AddFactor(ep, session, f);
    }
  }

  // add prior on first node

  // Initialize();
  // AddFactor();
  // AddNode();
  // GetNode();

  return (0);
}
