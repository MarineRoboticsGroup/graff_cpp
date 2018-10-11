#!/bin/sh
if [ ! -f include/json.hpp ]; then
    echo "JSON library not found; retrieving it."
    wget -q https://github.com/nlohmann/json/releases/download/v3.1.2/json.hpp
    mv json.hpp include/
    cd ..
fi

# if [ ! -d build ]; then
#     mkdir build
# fi

# cd build
# cmake ..
# make

# echo "Build complete."

