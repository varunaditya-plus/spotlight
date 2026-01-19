#!/bin/bash
set -e

eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv bash)"

cd "$(dirname "$0")"
mkdir -p build
cd build

echo "Configuring..."
cmake ..

echo "Building..."
cmake --build . -j$(nproc)

echo "Run with ./build/spotlight"
