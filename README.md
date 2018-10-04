# graff_cpp

## Introduction

A C++ interface to [SynchronySDK](https://github.com/nicrip/SynchronySDK_py) using JSON over ZeroMQ.

Communication with Synchrony is implemented via simple "request-reply"-type functions:

```c++
json request(zmq::socket_t &socket) {
  // create request
  json request;
  request["type"] = "GetStatus";
  // fill out the rest of the request here

  // serialize and send request, wait for reply
  zmq::message_t reply;
  if (sendRequest(request, socket, reply)) { 
    return (-1);
  }

  std::string r = toString(message);
  json reply = json::parse(r);
  return (reply);
}
```

To ensure compatibility, JSON requests must comply with Synchrony-defined entities ([example](https://github.com/GearsAD/SynchronySDK.jl/blob/master/src/entities/Session.jl)). These are also the reference when parsing replies.

## Getting Started

### Dependencies

 * `build-essential`
 * `cmake` (3.0.2+)
 * ZeroMQ - can be obtained using your package manager via `sudo apt install libzmq3-dev`
 * Niels Lohmann's [JSON library](https://github.com/nlohmann/json) - taken care of by the `configure.sh` script

### Build 

From the repository root:

```sh
mkdir build
cd build
cmake ..
make
```

### Test

Head over to [SynchronySDK_py](https://github.com/nicrip/SynchronySDK_py) and follow its setup instructions. Once complete, run `graff_server.py` - this will be your local proxy to the back-end.

Once the local proxy is running, try running the test application:

```sh
./build/bin/hexagonal_example
```

### Integration
TODO

### Releases
TODO

## Contribute

Contributions to this interface are welcome! Please fork this repository, add the desired functionality and submit a pull request.



