#!/bin/bash

# Vulkan Engine 1.3 Build Script

set -e  # Exit on any error

echo "🔨 Building Vulkan Engine 1.3..."

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "❌ Build directory not found. Please run setup.sh first."
    exit 1
fi

cd build

# Build the project
echo "📦 Compiling..."
make -j$(nproc)

echo "✅ Build completed successfully!"
echo ""
echo "To run the application:"
echo "  ./bin/VulkanEngine"
