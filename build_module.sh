#!/usr/bin/env bash
set -euo pipefail

if [ $# -lt 1 ]; then
    echo "Usage: $0 <module_name> [build|clean]"
    exit 1
fi

MODULE_NAME="$1"
COMMAND="${2:-build}"

ARCH="arm64"
CROSS_COMPILE="aarch64-linux-gnu-"

ROOT_DIR="$(pwd)"
KERNEL_DIR="$ROOT_DIR/kernel"
SRC_DIR="$ROOT_DIR/kernelModules"
BUILD_ROOT="$ROOT_DIR/build/modules"
MODULE_SRC="$SRC_DIR/${MODULE_NAME}.c"
MODULE_BUILD_DIR="$BUILD_ROOT/$MODULE_NAME"

if [ ! -d "$KERNEL_DIR" ]; then
    echo "Error: kernel directory not found: $KERNEL_DIR"
    exit 1
fi

if [ ! -d "$SRC_DIR" ]; then
    echo "Error: module source directory not found: $SRC_DIR"
    exit 1
fi

case "$COMMAND" in
    build)
        if [ ! -f "$MODULE_SRC" ]; then
            echo "Error: module source file not found: $MODULE_SRC"
            exit 1
        fi

        mkdir -p "$MODULE_BUILD_DIR"

        echo "Preparing build directory: $MODULE_BUILD_DIR"

        # Copy only the needed source into isolated build directory
        cp "$MODULE_SRC" "$MODULE_BUILD_DIR/"

        # Create Makefile inside build directory
        cat > "$MODULE_BUILD_DIR/Makefile" <<EOF
obj-m += ${MODULE_NAME}.o

KDIR := ${KERNEL_DIR}

all:
	\$(MAKE) -C \$(KDIR) M=\$(CURDIR) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

clean:
	\$(MAKE) -C \$(KDIR) M=\$(CURDIR) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} clean
EOF

        echo "Building module: $MODULE_NAME"
        make -C "$MODULE_BUILD_DIR" all -j"$(nproc)"

        echo
        echo "Build successful."
        echo "Artifacts:"
        echo "  $MODULE_BUILD_DIR/${MODULE_NAME}.ko"
        ;;
    clean)
        if [ -d "$MODULE_BUILD_DIR" ]; then
            if [ -f "$MODULE_BUILD_DIR/Makefile" ]; then
                echo "Cleaning kernel module outputs for: $MODULE_NAME"
                make -C "$MODULE_BUILD_DIR" clean || true
            fi

            echo "Removing build directory: $MODULE_BUILD_DIR"
            rm -rf "$MODULE_BUILD_DIR"
        else
            echo "Nothing to clean for module: $MODULE_NAME"
        fi
        ;;
    *)
        echo "Unknown command: $COMMAND"
        echo "Usage: $0 <module_name> [build|clean]"
        exit 1
        ;;
esac