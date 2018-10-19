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

  ToggleMockMode(ep);

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
      std::vector<double> var = {0.0001, 0.0, 0.0, 0.0,   0.0001,
                                 0.0,    0.0, 0.0, 0.0001};
      graff::Normal z_zpr(mean, var);
      graff::Factor zpr("Pose3PriorZPR", label, z_zpr);
      reply = AddFactor(ep, session, zpr);

      // add odometry (XYH measurement)
      std::string prev_label = "x" + std::to_string(idx - 1);
      var = {0.01, 0.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0, 0.0001};
      if (0 == j) {
        mean = {0.0, 0.0, 0.0}; // dive
      } else {
        mean = {0.0, direction * 1.0, 0.0}; // move sideways
      }
      graff::Normal z_xyh(mean, var);
      graff::Factor odometry("Pose3Pose3PartialXYH", {prev_label, label},
                             z_xyh);
      reply = AddFactor(ep, session, odometry);

      // add range measurements (121 total)
      int point_id(0);
      for (double z = -1.0; z <= 1.0; z += 0.2) {
        for (double y = -1.0; y <= 1.0; y += 0.2) {
          std::string pt =
              "p" + std::to_string(idx) + "_" + std::to_string(point_id);
          graff::Variable point(pt, "Point3");
          reply = AddVariable(ep, session, point);

          double az, el, r;
          az = atan2(y, 5.0);
          el = atan2(z, sqrt(pow(5.0, 2) + pow(y, 2)));
          r = sqrt(pow(5.0, 2) + pow(y, 2) + pow(z, 2));
          graff::Normal z_az(az, 0.0001);
          graff::Normal z_el(el, 0.0001);
          graff::Normal z_r(r, 0.01);
          graff::Factor range_measurement("RangeAzimuthElevation", {label, pt},
                                          {z_r, z_az, z_r});
          reply = AddFactor(ep, session, range_measurement);
          point_id++;
        }
      }

      // add a match constraint
      graff::Normal z_match({0.0, 0.0, 0.0},
                            {0.01, 0.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0, .01});
      std::string pt_a = "p" + std::to_string(idx-1) + "_60";
      std::string pt_b = "p" + std::to_string(idx) + "_55";
      graff::Factor match("Point3Point3", {pt_a, pt_b}, {z_match});
    }
  }

  // save session to disk
  json js = session.ToJson();
  std::ofstream o("pretty.json");
  o << std::setw(4) << js << std::endl;

  // set ready
  reply = RequestSolve(ep, session);

  std::this_thread::sleep_for(std::chrono::seconds(10));

  // get val

  // we're done here.
  RequestShutdown(ep);

  return (0);
}
