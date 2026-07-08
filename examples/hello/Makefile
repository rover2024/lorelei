# Build and run the hello thunk example with the devkit's LoreMakeThunk.py.
#
# Requires:
#   LORELEI_DEVKIT  an unpacked lorelei devkit for this host arch (provides LoreMakeThunk.py + the toolchain)
#   QEMU            the patched qemu-x86_64 (built with the dlcall plugin)
#   PLUGIN          the dlcall plugin, contrib/plugins/libdlcall.so from that build
#
#   make LORELEI_DEVKIT=/path/to/devkit QEMU=/path/to/qemu-x86_64 PLUGIN=/path/to/libdlcall.so run

DEVKIT ?= $(LORELEI_DEVKIT)
QEMU   ?=
PLUGIN ?=

HERE    := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
INSTALL := $(HERE)/install
GTL     := $(INSTALL)/x86_64/lib/x86_64-LoreGTL

.PHONY: all run clean

all: $(HERE)/build/main.elf

# 1. the native host library
$(HERE)/build/libhello.so: $(HERE)/hello.c
	mkdir -p $(HERE)/build
	cc -shared -fPIC -O2 -o $@ $<

# 2. generate + build the thunk (host HTL + guest GTL) with the devkit's LoreMakeThunk.py, into a
# self-contained thunk pack under install/ (itself a LORELEI_THUNK_PATH prefix).
$(GTL)/libhello.so: $(HERE)/build/libhello.so
	$(DEVKIT)/bin/LoreMakeThunk.py --devkit $(DEVKIT) --name hello \
	    --lib $(HERE)/build/libhello.so --header hello.h \
	    -o $(INSTALL) -- -I$(HERE)

# 3. the guest program, linked against the GTL
$(HERE)/build/main.elf: $(HERE)/main.c $(GTL)/libhello.so
	$(DEVKIT)/bin/x86_64-linux-gnu-clang $< -I$(HERE) -L$(GTL) -lhello -o $@

# 4. run the guest under qemu + the dlcall plugin. -L points qemu at the bundled x86_64 sysroot so it
# finds the guest's ld-linux/libc; on a non-x86_64 host that loader is otherwise absent, and on x86_64
# the flag is harmless (qemu only consults it for paths missing on the host).
run: all
	LORELEI_THUNK_PATH=$(INSTALL) \
	LD_LIBRARY_PATH=$(DEVKIT)/lib:$(INSTALL)/lib:$(HERE)/build \
	  $(QEMU) -L $(DEVKIT)/x86_64/sysroot -plugin $(PLUGIN) \
	  -E LD_LIBRARY_PATH=$(GTL):$(DEVKIT)/x86_64/lib \
	  $(HERE)/build/main.elf

clean:
	rm -rf $(HERE)/build $(INSTALL)
