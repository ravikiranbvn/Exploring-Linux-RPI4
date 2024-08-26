#!/bin/bash

# Set the cross-compiler and architecture
CROSS_COMPILE=aarch64-linux-gnu-
ARCH=arm64

# Paths
KERNEL_DIR=$(pwd)/kernel
HEADERS_DIR=$KERNEL_DIR/../kernel_headers
export KERNEL=kernel8

# Number of jobs for make (adjust based on your CPU cores)
JOBS=$(nproc)

# Function to build the kernel
build_kernel() {
    echo "Configuring the kernel for Raspberry Pi 4..."
    cd $KERNEL_DIR

    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE bcm2711_defconfig

    if [ $? -ne 0 ]; then
        echo "Kernel configuration failed!"
        exit 1
    fi

    echo "Kernel configured successfully."

    echo "Building the Linux kernel..."
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j$JOBS Image dtbs modules

    if [ $? -ne 0 ]; then
        echo "Kernel build failed!"
        exit 1
    fi

    echo "Kernel build completed successfully."

    echo "Installing kernel headers..."
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE headers_install INSTALL_HDR_PATH=$HEADERS_DIR

    if [ $? -ne 0 ]; then
        echo "Installing kernel headers failed!"
        exit 1
    fi

    echo "Kernel headers installed to $HEADERS_DIR."
}

# Function to clean the kernel build
clean_kernel() {
    echo "Cleaning the kernel build..."
    cd $KERNEL_DIR

    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE clean

    if [ $? -ne 0 ]; then
        echo "Kernel clean failed!"
        exit 1
    fi

    echo "Kernel build cleaned successfully."

    echo "Removing installed kernel headers..."
    rm -rf $HEADERS_DIR

    if [ $? -ne 0 ]; then
        echo "Failed to remove kernel headers directory!"
        exit 1
    fi

    echo "Kernel headers directory removed successfully."
}

# Main script logic
if [ "$1" == "clean" ]; then
    clean_kernel
elif [ "$1" == "build" ]; then
    build_kernel
else
    echo "Usage: $0 [build|clean]"
    exit 1
fi
