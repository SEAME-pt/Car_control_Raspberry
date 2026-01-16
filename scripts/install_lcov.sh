#!/usr/bin/env bash

set -euo pipefail

LCOV_TAG="v1.14"

echo "Installing lcov ${LCOV_TAG} from GitHub..."

echo "Updating apt package lists..."
sudo apt update

echo "Installing build dependencies..."
sudo apt install -y build-essential git wget perl ca-certificates || true

echo "Removing any distro lcov package to avoid conflicts..."
sudo apt remove -y lcov || true

TMPDIR=$(mktemp -d)
cleanup() {
	rm -rf "$TMPDIR"
}
trap cleanup EXIT

cd "$TMPDIR"
echo "Cloning lcov repository..."
git clone --depth 1 --branch ${LCOV_TAG} https://github.com/linux-test-project/lcov.git lcov-src
cd lcov-src

echo "Installing lcov ${LCOV_TAG}..."
sudo make install

echo "Installation complete. Verifying versions..."
echo "lcov:" && lcov --version || true
echo "gcov:" && gcov --version || true

echo "✔ lcov ${LCOV_TAG} installed"

