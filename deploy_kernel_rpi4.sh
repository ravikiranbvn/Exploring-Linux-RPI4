#!/usr/bin/env bash
set -euo pipefail

# -----------------------------
# Config
# -----------------------------
PI_HOST="pi-home"
PI_USER="pi"
KERNEL_IMAGE_NAME="kernel8.img"
BOOT_DIR_REMOTE="/boot/firmware"
STAGING_DIR="$(pwd)/.deploy_rpi4"

# Kernel build settings
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
export KERNEL=kernel8

KERNEL_DIR="$(pwd)/kernel"

# -----------------------------
# Checks
# -----------------------------
if [ ! -d "$KERNEL_DIR" ]; then
    echo "ERROR: kernel/ directory not found"
    exit 1
fi

if [ ! -f "$KERNEL_DIR/arch/arm64/boot/Image" ]; then
    echo "ERROR: kernel image not found. Build kernel first."
    echo "Expected: $KERNEL_DIR/arch/arm64/boot/Image"
    exit 1
fi

# -----------------------------
# Prepare staging
# -----------------------------
echo "[1/6] Preparing staging area..."
rm -rf "$STAGING_DIR"
mkdir -p "$STAGING_DIR/rootfs"
mkdir -p "$STAGING_DIR/boot/overlays"

# -----------------------------
# Install modules to staging
# -----------------------------
echo "[2/6] Installing modules to staging..."
make -C "$KERNEL_DIR" \
    ARCH="$ARCH" \
    CROSS_COMPILE="$CROSS_COMPILE" \
    INSTALL_MOD_PATH="$STAGING_DIR/rootfs" \
    modules_install

# -----------------------------
# Collect boot files
# -----------------------------
echo "[3/6] Collecting kernel image, DTBs, overlays..."
cp "$KERNEL_DIR/arch/arm64/boot/Image" \
   "$STAGING_DIR/boot/${KERNEL_IMAGE_NAME}"

cp "$KERNEL_DIR"/arch/arm64/boot/dts/broadcom/*.dtb \
   "$STAGING_DIR/boot/"

cp "$KERNEL_DIR"/arch/arm64/boot/dts/overlays/*.dtb* \
   "$STAGING_DIR/boot/overlays/"

cp "$KERNEL_DIR"/arch/arm64/boot/dts/overlays/README \
   "$STAGING_DIR/boot/overlays/"

# -----------------------------
# Transfer to Pi
# -----------------------------
echo "[4/6] Copying files to ${PI_USER}@${PI_HOST}..."
ssh "${PI_USER}@${PI_HOST}" "mkdir -p /tmp/kernel_deploy/overlays /tmp/kernel_deploy/modules"

rsync -av "$STAGING_DIR/rootfs/lib/modules/" \
    "${PI_USER}@${PI_HOST}:/tmp/kernel_deploy/modules/"

scp "$STAGING_DIR/boot/${KERNEL_IMAGE_NAME}" \
    "${PI_USER}@${PI_HOST}:/tmp/kernel_deploy/"

scp "$STAGING_DIR/boot/"*.dtb \
    "${PI_USER}@${PI_HOST}:/tmp/kernel_deploy/"

scp -r "$STAGING_DIR/boot/overlays/"* \
    "${PI_USER}@${PI_HOST}:/tmp/kernel_deploy/overlays/"

# -----------------------------
# Install on Pi
# -----------------------------
echo "[5/6] Installing on Pi..."
ssh -t "${PI_USER}@${PI_HOST}" <<'EOF'
set -euo pipefail

BOOT_DIR="/boot/firmware"
DEPLOY_DIR="/tmp/kernel_deploy"

echo "Backing up current kernel image..."
sudo cp "${BOOT_DIR}/kernel8.img" "${BOOT_DIR}/kernel8-backup.img" || true

echo "Installing new kernel image..."
sudo cp "${DEPLOY_DIR}/kernel8.img" "${BOOT_DIR}/"

echo "Installing DTBs..."
sudo cp "${DEPLOY_DIR}/"*.dtb "${BOOT_DIR}/"

echo "Installing overlays..."
sudo cp "${DEPLOY_DIR}/overlays/"* "${BOOT_DIR}/overlays/"

echo "Installing modules..."
sudo cp -a "${DEPLOY_DIR}/modules/"* /lib/modules/

echo "Running depmod..."
sudo depmod -a

echo "Syncing..."
sync

echo "Kernel deployment complete."
EOF

# -----------------------------
# Reboot prompt
# -----------------------------
echo "[6/6] Done."
echo "Reboot the Pi with:"
echo "  ssh ${PI_USER}@${PI_HOST} 'sudo reboot'"