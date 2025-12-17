#!/bin/bash

set -e

echo "Updating system..."
sudo apt update

echo "Installing libevdev..."
sudo apt install -y \
    libevdev-dev

echo "Cleaning..."
sudo apt autoremove -y
sudo apt autoclean

echo "Done! SDL2 successfully installed."
