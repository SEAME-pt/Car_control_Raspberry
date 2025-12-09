#!/bin/bash

set -e

echo "Updating system..."
sudo apt update

echo "Installing required tools..."
sudo apt install -y build-essential cmake git

echo "Cloning GoogleTest..."
git clone https://github.com/google/googletest.git
cd googletest

echo "Configuring build..."
cmake -B build -DCMAKE_BUILD_TYPE=Release

echo "Compiling..."
cmake --build build

echo "Installing..."
sudo cmake --install build

echo "Refreshing linker cache..."
sudo ldconfig

echo "Cleaning..."
cd ..
rm -rf googletest

echo "✔ Google Test successfully installed!"
