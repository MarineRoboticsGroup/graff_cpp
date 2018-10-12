#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <graff/graff.hpp>

int main(int argCount, char **argValues) {
  graff::Endpoint ep;

  std::cout << "Connecting to endpoint…";
  ep.Connect("tcp://127.0.0.1:5555");
  std::cout << "connected!" << std::endl;

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
    std::string idx = "x" + std::to_string(i);
    graff::Variable pose(idx, "Pose2");
    reply = AddVariable(ep, session, pose);

    if (i > 0) {
      std::string prev_idx = "x" + std::to_string(i - 1);
      std::vector<double> mu = {10.0, 0.0, PI / 3.0};
      std::vector<double> sig = {0.01, 0.0, 0.0, 0.0, 0.01,
                                 0.0,  0.0, 0.0, 0.01};
      graff::Normal z(mu, sig);
      std::vector<std::string> nodes = {prev_idx, idx};
      graff::Factor odometry("Pose2Pose2", nodes, z);
      reply = AddFactor(ep, session, odometry);
    }
  }

  // add prior on first node
  std::vector<double> mean = {0.0, 0.0, 0.0};
  std::vector<double> cov = {0.01, 0.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0, 0.01};
  graff::Normal p0(mean, cov);
  graff::Factor prior0("PriorPose2", "x0", p0);
  reply = AddFactor(ep, session, prior0);

  // add landmark
  graff::Variable l1("l1", "Point2");
  AddVariable(ep, session, l1);

  // add first landmark observation
  graff::Normal zb1(0, 0.1);
  graff::Normal zr1(10, 1.0);
  std::vector<graff::Distribution> z1 = {zb1, zr1};
  graff::Factor f1("Pose2Point2BearingRange",
                   std::vector<std::string>({"x0", "l1"}), z1);
  reply = AddFactor(ep, session, f1);

  // add second landmark observation
  graff::Normal zb2(0, 0.1);
  graff::Normal zr2(20, 1.0);
  std::vector<graff::Distribution> z2 = {zb2, zr2};
  graff::Factor f2("Pose2Point2BearingRange",
                   std::vector<std::string>({"x6", "l1"}), z2);
  reply = AddFactor(ep, session, f2);

  reply = RequestSolve(ep, session);

  std::this_thread::sleep_for(std::chrono::seconds(10));

  // get

  return (0);
}
