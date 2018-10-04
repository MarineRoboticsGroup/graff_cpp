#!/bin/sh
if [ ! -f include/json.hpp ]; then
    echo "JSON library not found; retrieving it."
    mkdir include
    cd include
    wget -q https://github.com/nlohmann/json/releases/download/v3.1.2/json.hpp
    cd ..
fi

if [ ! -d build ]; then
    mkdir build
fi

cd build
cmake ..
make

echo "Build complete."

