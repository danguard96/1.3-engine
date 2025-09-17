#!/bin/bash

# Vulkan Engine 1.3 Setup Script
# This script sets up the project dependencies and builds the project

set -e  # Exit on any error

echo "🚀 Setting up Vulkan Engine 1.3..."

# Check if Python 3 is available
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is required but not installed"
    exit 1
fi

# Check if Git is available
if ! command -v git &> /dev/null; then
    echo "❌ Git is required but not installed"
    exit 1
fi

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake is required but not installed"
    exit 1
fi

# Check if Vulkan SDK is available
if ! command -v glslc &> /dev/null; then
    echo "❌ Vulkan SDK is required but not installed or not in PATH"
    echo "Please install Vulkan SDK and add glslc to your PATH"
    exit 1
fi

echo "✅ Prerequisites check passed"

# Run bootstrap script
echo "📦 Downloading dependencies..."
cd deps
python3 bootstrap.py
cd ..

# Copy header-only libraries
echo "📋 Copying header files..."
python3 scripts/copy_headers.py

# Create build directory
echo "🔨 Setting up build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "⚙️  Configuring with CMake..."
cmake .. -G "Unix Makefiles"

echo "✅ Setup completed successfully!"
echo ""
echo "To build the project, run:"
echo "  cd build"
echo "  make"
echo ""
echo "To run the application:"
echo "  ./bin/VulkanEngine"
