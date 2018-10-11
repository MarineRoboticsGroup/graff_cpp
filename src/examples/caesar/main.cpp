#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <graff/graff.hpp>

int main(int argCount, char **argValues) {
  graff::Endpoint ep;

  std::cout << "Connecting to endpoint…" << std::endl;
  ep.Connect("tcp://127.0.0.1:5555");
  std::cout << "Connected!" << std::endl;

  graff::Robot robot("krakenoid");
  graff::Session session("first dive");

  json reply;
  reply = RegisterRobot(ep, robot);
  std::cout << "Registered robot " << robot.name() << std::endl;
  if (check(reply)) {
    std::cout << "success!\n";
  }
  reply = RegisterSession(ep, robot, session);
  std::cout << "Registered session " << session.name() << std::endl;
  if (check(reply)) {
    std::cout << "success!\n";
  }

  for (int i = 0; i < 6; ++i) {
    std::string name;
    name = "x" + std::to_string(i);
    graff::Pose2 n(name);

    reply = AddVariable(ep, session, n); // TODO check if successful

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
  // AddVariable();
  // GetNode();

  return (0);
}
