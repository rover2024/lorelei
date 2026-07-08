# Build and run the demo thunk example with the devkit's LoreMakeThunk.py.
#
#   DEVKIT   an unpacked lorelei devkit (provides LoreMakeThunk.py and the guest toolchain)
#   QEMU     the patched qemu-x86_64 (built with the dlcall plugin), used by `make run`
#   PLUGIN   the dlcall plugin libdlcall.so, used by `make run`
#
#   export DEVKIT=/path/to/devkit
#   make                              # build
#   make QEMU=... PLUGIN=... run      # build and run under qemu

DEVKIT ?= $(LORELEI_DEVKIT)
QEMU   ?=
PLUGIN ?=

HERE    := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
INSTALL := $(HERE)/install
GTL     := $(INSTALL)/x86_64/lib/x86_64-LoreGTL

.PHONY: all run clean

all: $(HERE)/build/main.elf

# 1. the native host library
$(HERE)/build/libdemo.so: $(HERE)/demo.c
	mkdir -p $(HERE)/build
	cc -shared -fPIC -O2 -o $@ $<

# 2. generate + build the thunk (host HTL + guest GTL) with the devkit's LoreMakeThunk.py, into a
# self-contained thunk pack under install/. LoreMakeThunk finds its devkit from its own location
# ($(DEVKIT)/bin), so no --devkit is needed.
$(GTL)/libdemo.so: $(HERE)/build/libdemo.so
	$(DEVKIT)/bin/LoreMakeThunk.py --name demo --lib $< --header demo.h -o $(INSTALL) -- -I$(HERE)

# 3. the guest program, linked against the GTL
$(HERE)/build/main.elf: $(HERE)/main.c $(GTL)/libdemo.so
	$(DEVKIT)/bin/x86_64-linux-gnu-clang $< -I$(HERE) -L$(GTL) -ldemo -o $@

# 4. run the guest under qemu + the dlcall plugin. -L points qemu at the bundled x86_64 sysroot so it
# finds the guest's ld-linux/libc. On a non-x86_64 host that loader is otherwise absent, and on x86_64
# the flag is harmless (qemu only consults it for paths missing on the host).
run: all
	LORELEI_THUNK_PATH=$(INSTALL) \
	LD_LIBRARY_PATH=$(DEVKIT)/lib:$(HERE)/build \
	  $(QEMU) -L $(DEVKIT)/x86_64/sysroot -plugin $(PLUGIN) \
	  -E LD_LIBRARY_PATH=$(DEVKIT)/x86_64/lib:$(GTL) \
	  $(HERE)/build/main.elf

clean:
	rm -rf $(HERE)/build $(INSTALL)
