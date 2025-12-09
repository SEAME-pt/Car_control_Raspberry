#!/bin/bash

set -e

# Update system
echo "Updating system..."
sudo apt update

# Install dependencies
echo "Installing required packages..."
sudo apt install -y build-essential perl

# Install stable version from apt
echo "Installing lcov via apt..."
sudo apt install -y lcov

# Verify installation
echo "Verifying installation..."
lcov --version

echo "✔ lcov successfully installed!"