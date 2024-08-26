#!/bin/bash

# Check if a module name is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <module_name> [build|clean]"
    exit 1
fi

MODULE_NAME=$1
COMMAND=${2:-build}  # Default command is 'build' if not provided

# Set the cross-compiler and architecture
CROSS_COMPILE=aarch64-linux-gnu-
ARCH=arm64

# Paths
KERNEL_DIR=$(pwd)/kernel  # Path to the full kernel source directory
MODULE_DIR=$(pwd)/kernelModules  # Path to the custom module directory

# Check if the module source file exists (only when building)
if [ ! -f "$MODULE_DIR/$MODULE_NAME.c" ] && [ "$COMMAND" != "clean" ]; then
    echo "Error: Module source file $MODULE_DIR/$MODULE_NAME.c does not exist."
    exit 1
fi

# Create the Makefile dynamically
echo "Creating Makefile for $MODULE_NAME..."

cat <<EOL > $MODULE_DIR/Makefile
# Name of the module
obj-m += $MODULE_NAME.o

# Path to the kernel headers or build directory
KDIR := $KERNEL_DIR  # Use the full kernel source directory

# Command to build the module
all:
	\$(MAKE) -C \$(KDIR) M=\$(PWD) modules

# Command to clean the build files
clean:
	\$(MAKE) -C \$(KDIR) M=\$(PWD) clean
EOL

# Execute the build or clean command
cd $MODULE_DIR

if [ "$COMMAND" == "build" ]; then
    echo "Building the $MODULE_NAME module..."
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE -j$(nproc)

    if [ $? -ne 0 ]; then
        echo "Module build failed!"
        exit 1
    fi

    echo "Module build completed successfully."

elif [ "$COMMAND" == "clean" ]; then
    echo "Cleaning the $MODULE_NAME module..."
    make ARCH=$ARCH CROSS_COMPILE=$CROSS_COMPILE clean

    if [ $? -ne 0 ]; then
        echo "Module clean failed!"
        exit 1
    fi

    echo "Module clean completed successfully."

else
    echo "Unknown command: $COMMAND"
    echo "Usage: $0 <module_name> [build|clean]"
    exit 1
fi
