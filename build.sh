#!/bin/bash
# build.sh - Automated CMake build & test script for ctoml

set -e  # Stop on any error

BUILD_DIR="build"
BUILD_TYPE="${BUILD_TYPE:-Debug}"  # Default Debug, override with BUILD_TYPE=Release ./build.sh

echo "🚀 Building ctoml ($BUILD_TYPE)..."

# Clean previous build
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# Build
cmake --build . --parallel $(nproc)  # Use all CPU cores

# Run tests
echo "🧪 Running tests..."
ctest -V -j$(nproc) --output-on-failure # Verbose, parallel

echo "✅ Build & tests PASSED!"
echo "📁 Test binary: $(pwd)/tests/test_ctoml"

# Run test manually with ASan leak detection
# echo "🔍 Running with ASan leak check..."
# Not supported on macOS due to missing leak detection in ASan, but on Linux you can uncomment this line to check for leaks:
# ASAN_OPTIONS=detect_leaks=1 ./tests/test_ctoml

cd ..
