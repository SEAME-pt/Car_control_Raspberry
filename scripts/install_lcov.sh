#!/bin/bash

set -e

# Update system
echo "Updating system..."
sudo apt update

# Install dependencies
echo "Installing required packages..."
sudo apt install -y build-essential perl git

# Install gcov (should already be installed with gcc)
echo "Ensuring gcov is available..."
sudo apt install -y gcc-13

# Remove old lcov if installed
echo "Removing old lcov version if exists..."
sudo apt remove -y lcov || true

# Download and install lcov 2.0-1
echo "Downloading lcov 2.0-1..."
cd /tmp
wget https://github.com/linux-test-project/lcov/releases/download/v2.0/lcov-2.0-1.tar.gz

echo "Extracting lcov 2.0-1..."
tar -xzf lcov-2.0-1.tar.gz
cd lcov-2.0-1

echo "Installing lcov 2.0-1..."
sudo make install

# Clean up
cd /tmp
rm -rf lcov-2.0-1 lcov-2.0-1.tar.gz

# Verify installation
echo "Verifying installation..."
echo "lcov version:"
lcov --version
echo ""
echo "gcov version:"
gcov --version

echo "✔ lcov 2.0-1 successfully installed!"