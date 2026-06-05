export PATH := /buildroot/output/host/usr/sbin$\
	:/buildroot/output/host/usr/bin/$\
	:/buildroot/output/host/sbin$\
	:/buildroot/output/host/bin/$\
	:$(PATH)

MODULE := temp_regulator
SOURCE := main

# Part executed when called from kernel build system:
ifneq ($(KERNELRELEASE),)
obj-m += $(MODULE).o		## name of the generated module

# add folder sources
SOURCES_C := $(wildcard $(src)/*.c)
# add temperature sources
SOURCES_C += $(wildcard $(src)/temperature/*.c)


$(MODULE)-objs := $(patsubst $(src)/%.c, %.o, $(SOURCES_C))

# $(MODULE)-objs := $(SOURCE).o	## list of objects needed for that module
CFLAGS_$(SOURCE).o := -DDEBUG

# Part executed when called from standard make in module source directory:
else
include ../../kernel_settings
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(CPU) CROSS_COMPILE=$(TOOLS) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	echo $(PATH)

install:
	$(MAKE) -C $(KDIR) M=$(PWD) INSTALL_MOD_PATH=$(MODPATH) modules_install
	# install -D -m 0644 $(MODULE).conf $(MODPATH)/etc/modprobe.d/$(MODULE).conf

endif
