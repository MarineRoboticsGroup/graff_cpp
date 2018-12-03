#include <chrono>
#include <fstream>
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
  std::cout << "Registering robot " << robot.Name();
  reply = graff::RegisterRobot(ep, robot);
  if (check(reply)) {
    std::cout << " - success!\n";
  }
  std::cout << "Registering session " << session.name();
  reply = graff::RegisterSession(ep, robot, session);
  if (check(reply)) {
    std::cout << " - success!\n";
  }

  for (int i = 0; i < 6; ++i) {
    std::string idx = "x" + std::to_string(i);
    graff::Variable pose(idx, "Pose2");
    reply = graff::AddVariable(ep, session, pose);

    if (i > 0) {
      std::string prev_idx = "x" + std::to_string(i - 1);
      std::vector<double> mu = {10.0, 0.0, PI / 3.0};
      std::vector<double> sig = {0.01, 0.0, 0.0, 0.0, 0.01,
                                 0.0,  0.0, 0.0, 0.01};
      graff::Normal *z = new graff::Normal(mu, sig);
      std::vector<std::string> nodes = {prev_idx, idx};
      graff::Factor odometry("Pose2Pose2", nodes);
      odometry.push_back(z); // measurement
      reply = graff::AddFactor(ep, session, odometry);
    }
  }

  // add prior on first node
  std::vector<double> mean = {0.0, 0.0, 0.0};
  std::vector<double> cov = {0.01, 0.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0, 0.01};
  graff::Normal *p0 = new graff::Normal(mean, cov);
  graff::Factor prior0("Prior"); //, "x0", p0);
  prior0.push_back("x0");
  prior0.push_back(p0);
  reply = graff::AddFactor(ep, session, prior0);

  // add landmark
  graff::Variable l1("l1", "Point2");
  graff::AddVariable(ep, session, l1);

  // add first landmark observation
  graff::Normal *zb1 = new graff::Normal(0, 0.1);
  graff::Normal *zr1 = new graff::Normal(10, 1.0);
  graff::Factor f1("Pose2Point2BearingRange",
                   std::vector<std::string>({"x0", "l1"}));
  f1.push_back(zb1);
  f1.push_back(zr1);
  reply = graff::AddFactor(ep, session, f1);

  // add second landmark observation
  graff::Normal *zb2 = new graff::Normal(0, 0.1);
  graff::Normal *zr2 = new graff::Normal(20, 1.0);
  graff::Factor f2("Pose2Point2BearingRange",
                   std::vector<std::string>({"x6", "l1"}));
  f2.push_back(zb2);
  f2.push_back(zr2);
  reply = graff::AddFactor(ep, session, f2);

  // save session to disk
  json js = session.ToJson();
  std::ofstream o("hexagonal.json");
  o << std::setw(4) << js << std::endl;

  reply = graff::RequestSolve(ep, session);

  std::this_thread::sleep_for(std::chrono::seconds(10));

  return (0);
}
