#!/bin/bash

# Set the cross-compiler and architecture
CROSS_COMPILE=aarch64-linux-gnu-
ARCH=arm64

# Paths
KERNEL_DIR=$(pwd)/kernel
HEADERS_DIR=$KERNEL_DIR/../kernel_headers

# Number of jobs for make (adjust based on your CPU cores)
JOBS=$(nproc)

# Step 1: Build the Linux kernel
echo "Building the Linux kernel..."
cd $KERNEL_DIR

make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j$JOBS Image dtbs modules modules_prepare

if [ $? -ne 0 ]; then
    echo "Kernel build failed!"
    exit 1
fi

echo "Kernel build completed successfully."

# Step 2: Install kernel headers (for module development)
echo "Installing kernel headers..."
make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE headers_install INSTALL_HDR_PATH=$HEADERS_DIR

echo "Kernel headers installed to $HEADERS_DIR."
