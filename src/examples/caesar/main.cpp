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
  std::cout << "Registering robot " << robot.name();
  reply = RegisterRobot(ep, robot);
  if (check(reply)) {
    std::cout << " - success!\n";
  }
  std::cout << "Registering session " << session.name();
  reply = RegisterSession(ep, robot, session);
  if (check(reply)) {
    std::cout << " - success!\n";
  }

  for (int i = 0; i < 6; ++i) {
    std::string name;
    name = "x" + std::to_string(i);
    graff::Pose2 n(name);

    reply = AddVariable(ep, session, n); // TODO check if successful

    // add odometry
    if (i > 0) {
      std::string start, stop;
      start = "x" + std::to_string(i - 1);
      stop = "x" + std::to_string(i);
      std::vector<double> mu = {1.0, 0.0, PI / 6.0};
      std::vector<std::vector<double>> sig = {
          {0.01, 0.0, 0.0}, {0.0, 0.01, 0.0}, {0.0, 0.0, 0.01}};
      graff::MvNormal z(mu, sig);
      std::vector<std::string> nodes = {start, stop};
      graff::Factor f("Pose2Pose2", nodes, z);
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
