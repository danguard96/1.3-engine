#!/bin/bash

# Vulkan Engine 1.3 Clean Script

echo "🧹 Cleaning Vulkan Engine 1.3..."

# Remove build directory
if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
fi

# Remove external directory
if [ -d "external" ]; then
    echo "Removing external directory..."
    rm -rf external
fi

# Remove deps/src directory
if [ -d "deps/src" ]; then
    echo "Removing dependencies source directory..."
    rm -rf deps/src
fi

echo "✅ Clean completed successfully!"
echo ""
echo "To setup the project again, run:"
echo "  ./setup.sh"
