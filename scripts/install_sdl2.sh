#!/bin/bash

set -e

echo "Updating system..."
sudo apt update

echo "Installing SDL2 core and extensions..."
sudo apt install -y \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-ttf-dev \
    libsdl2-net-dev \
    libsdl2-gfx-dev

echo "Cleaning..."
sudo apt autoremove -y
sudo apt autoclean

echo "Done! SDL2 successfully installed."
