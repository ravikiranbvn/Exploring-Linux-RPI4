# Exploring-Linux-RPI4

Minimal workspace for:

* Raspberry Pi kernel (submodule)
* Out-of-tree kernel modules
* Cross-compilation workflow

---

## Structure

```
.
├── kernel/           # RPi Linux (submodule)
├── kernel_headers/   # Exported headers (userspace only)
├── modules/          # Custom modules
├── build_kernel.sh
└── build_module.sh
```

---

## Update Kernel

```
git submodule sync
git submodule update --init --remote kernel
```

---

## Prepare Kernel (REQUIRED for modules)

```
cd kernel
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- modules_prepare
```

---

## Build Module

```
./build_module.sh <module_name> build
```

---

## Key Rule

* Use `kernel/` → for module builds
* Do NOT use `kernel_headers/` for modules

---

## (Optional) Full Kernel Build

```
./build_kernel.sh build
```

---

## Fix: Detached HEAD in submodule

```
cd kernel
git checkout -B rpi-6.12.y origin/rpi-6.12.y
```

---

## Toolchain

```
sudo apt install gcc-aarch64-linux-gnu
```

## References

Build and copy kernel:
- https://www.raspberrypi.com/documentation/computers/linux_kernel.html

Kernel modules inspired from: 
- https://github.com/Johannes4Linux/Linux_Driver_Tutorial/tree/main
- https://github.com/Embetronicx/Tutorials/tree/master/Linux/Device_Driver

Pinout reference: 
- https://pinout.xyz/
