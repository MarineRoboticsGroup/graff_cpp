# graff_cpp

[![Build Status](https://travis-ci.org/pvazteixeira/graff_cpp.svg?branch=master)](https://travis-ci.org/pvazteixeira/graff_cpp)

## Overview

A C++ interface to the [Caesar.jl framework](https://github.com/JuliaRobotics/Caesar.jl) using JSON over ZeroMQ.
Communication with the Caesar endpoint currently follows a "request-reply" pattern.

## Usage

The first step is to establish a connection to the [Caesar endpoint](), and registering a new session: 

```c++
  graff::Endpoint ep;
  ep.Connect("tcp://127.0.0.1:5555");

  graff::Robot robot("krakenoid");
  graff::Session session("first dive");

  json reply;
  reply = graff::RegisterRobot(ep, robot);
  reply = graff::RegisterSession(ep, robot, session);
```

Syntax is similar to that of other libraries, such as [isam](https://people.csail.mit.edu/kaess/isam/), using dumb/shallow variables such as `graff::Variable`, `graff::Factor`, and `graff::Distribution` (or one of its derived types, such as `graff::Normal`):

```c++
  for (int i = 0; i < 6; ++i) {
    std::string idx = "x" + std::to_string(i);
    graff::Variable pose(idx, "Pose2");
    reply = graff::AddVariable(ep, session, pose);

    if (i > 0) {
      std::string prev_idx = "x" + std::to_string(i - 1);
      std::vector<double> mu = {10.0, 0.0, PI / 3.0};
      std::vector<double> sig = {0.01, 0.0, 0.0, 0.0, 0.01,
                                 0.0,  0.0, 0.0, 0.01};
      graff::Normal z(mu, sig);
      std::vector<std::string> nodes = {prev_idx, idx};
      graff::Factor odometry("Pose2Pose2", nodes, z);
      reply = graff::AddFactor(ep, session, odometry);
    }
  }
```

As an additional step, you must specify when the graph is ready to be solved:

```c++
```

The endpoint can then be queried for estimates:

```c++
```


## Installation 

### Dependencies

 * [ZeroMQ](http://zeromq.org) - can be obtained using your package manager via `sudo apt install libzmq3-dev`
 * Niels Lohmann's [JSON library](https://github.com/nlohmann/json) - taken care of by the `configure.sh` script
 * C++11 (gcc 4.9+ or clang 3.5+)
 * `build-essential`
 * `cmake` (3.0.2+)

### Build 

From the repository root:

```sh
./configure.sh 
mkdir build
cd build
cmake ..
make
```

### Test

Head over to [Caesar.jl](https://github.com/JuliaRobotics/Caesar.jl) and follow its setup instructions. Once complete, run `graff_server.py` - this will be your local proxy to the back-end.

Once the local proxy is running, try running the test application:

```sh
./build/bin/caesar_hexagonal
```

### Integration
TODO

### Releases
TODO

## Contribute

Contributions to this interface are welcome! Please fork this repository, add the desired functionality, and submit a pull request.



