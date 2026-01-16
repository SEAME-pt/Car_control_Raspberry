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

# Ensure gcc-11 / gcov-11 (11.4) is installed
echo "Checking for gcov-11 (11.4)..."
if command -v gcov-11 >/dev/null 2>&1; then
	current_gcov=$(gcov-11 --version | head -n1 | sed -n 's/.*\([0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/p')
	echo "Found gcov-11 version: ${current_gcov}"
fi

if [ -z "${current_gcov-}" ] || [[ "${current_gcov}" != 11.4* ]]; then
	echo "Attempting to install gcc-11/g++-11 (prefer version 11.4)"
	sudo apt update || true
	# Look for available gcc-11 versions matching 11.4
	candidate=$(apt-cache madison gcc-11 | awk '{print $3}' | grep '^11\.4' | head -n1 || true)
	if [ -n "$candidate" ]; then
		echo "Installing specific package version: $candidate"
		sudo apt install -y gcc-11="$candidate" g++-11="$candidate" || sudo apt install -y gcc-11 g++-11
	else
		echo "Specific 11.4 package not found in apt, installing latest gcc-11/g++-11"
		sudo apt install -y gcc-11 g++-11 || true
	fi

	if command -v gcov-11 >/dev/null 2>&1; then
		current_gcov=$(gcov-11 --version | head -n1 | sed -n 's/.*\([0-9]\+\.[0-9]\+\.[0-9]\+\).*/\1/p')
		echo "After installation, gcov-11 version: ${current_gcov}"
	else
		echo "Warning: gcov-11 not found after attempted installation"
	fi
fi

# Configure gcov alternative to point to gcov-11
if command -v gcov-11 >/dev/null 2>&1; then
	echo "Configuring /usr/bin/gcov to point to gcov-11"
	sudo update-alternatives --install /usr/bin/gcov gcov /usr/bin/gcov-11 1100 || true
	sudo update-alternatives --set gcov /usr/bin/gcov-11 || true
fi

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

