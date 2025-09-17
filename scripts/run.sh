#!/bin/bash

# Vulkan Engine 1.3 Run Script

echo "🚀 Running Vulkan Engine 1.3..."

# Check if executable exists
if [ ! -f "build/bin/VulkanEngine" ]; then
    echo "❌ Executable not found. Please build the project first."
    echo "Run: ./scripts/build.sh"
    exit 1
fi

# Run the application
cd build
./bin/VulkanEngine
