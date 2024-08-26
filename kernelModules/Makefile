# Name of the module
obj-m += hello_world.o

# Path to the kernel headers or build directory
KDIR := /home/ravi/dw/repos/Exploring-Linux-RPI4/kernel  # Use the full kernel source directory

# Command to build the module
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Command to clean the build files
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
