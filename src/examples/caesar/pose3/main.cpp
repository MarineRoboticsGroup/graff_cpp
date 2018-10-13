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

  graff::Robot robot("krakenoid3000");
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

  double direction(1.0), depth(0.0);

  graff::Variable pose("x0", "Pose3");
  reply = AddVariable(ep, session, pose);
  std::vector<double> mean = {0.0, 0.0, 0.0};
  std::vector<double> cov = {0.01, 0.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0, 0.01};
  graff::Normal p0(mean, cov);
  graff::Factor prior0("PriorPose2", "x0", p0);
  reply = AddFactor(ep, session, prior0);

  // vertical lawn-mower, each leg is at constant depth
  for (int i = 0; i < 3; ++i) {
    direction *= -1.0; //
    depth += 1.0;      // increase direction by 1m after each leg
    for (int j = 0; j < 10; ++j) {
      int idx = i * 10 + j + 1;

      // add pose
      std::string label = "x" + std::to_string(idx);
      graff::Variable pose(label, "Pose3");
      reply = AddVariable(ep, session, pose);

      // add ZPR prior
      std::vector<double> mean = {0.0, 0.0, 0.0};
      std::vector<double> var = {0.0001, 0.0001, 0.0001};
      graff::Normal z_zpr(mean, var);
      graff::Factor zpr("Pose3PriorZPR", label, z_zpr);
      reply = AddFactor(ep, session, zpr);

      // add odometry (XYH measurement)
      std::string prev_label = "x" + std::to_string(idx - 1);
      var = {0.01, 0.01, 0.0001};
      if (0 == j) {
        mean = {0.0, 0.0, 0.0}; // dive
      } else {
        mean = {0.0, direction * 10.0, 0.0}; // move sideways
      }
      graff::Normal z_xyh(mean, var);
      graff::Factor odometry("Pose3Pose3PartialXYH", {prev_label, label},
                             z_xyh);
      reply = AddFactor(ep, session, odometry);

      // TODO: add range measurements

    }
  }

  // save session to disk
  json js = session.ToJson();
  std::ofstream o("pretty.json");
  o << std::setw(4) << js << std::endl;

  reply = RequestSolve(ep, session);

  std::this_thread::sleep_for(std::chrono::seconds(10));

  return (0);
}
