# Build and run the demo thunk example. Like hello it builds a guest libdemo.so (that main links) and
# a host libdemo.so (that the thunk dispatches to), from one source. Unlike hello, demo's output does
# not reveal which one ran (it forwards the real work to libc), so `make run` shows the thunked run:
# the variadic and callback calls carried across to the host.
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

HERE   := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
SRC    := $(HERE)/src
GTL    := $(HERE)/thunks/x86_64

.PHONY: all run clean

all: $(HERE)/build/guest/main $(GTL)/libdemo.so

# 1. the guest library (x86_64) and the guest program. main links the guest library but carries no
# rpath, so which libdemo.so it loads is decided entirely by LD_LIBRARY_PATH at run time.
$(HERE)/build/guest/main: $(SRC)/demo.c $(SRC)/main.c $(SRC)/demo.h
	mkdir -p $(HERE)/build/guest
	$(DEVKIT)/bin/x86_64-linux-gnu-clang -shared -fPIC $(SRC)/demo.c -o $(HERE)/build/guest/libdemo.so
	$(DEVKIT)/bin/x86_64-linux-gnu-clang $(SRC)/main.c -I$(SRC) -L$(HERE)/build/guest -ldemo -o $@

# 2. the host library, native to this host, from the same source, for the host thunk to dispatch to.
$(HERE)/build/host/libdemo.so: $(SRC)/demo.c $(SRC)/demo.h
	mkdir -p $(HERE)/build/host
	cc -shared -fPIC $(SRC)/demo.c -o $@

# 3. the thunk, generated from the host library into a self-contained thunk pack under thunks/.
# LoreMakeThunk finds its devkit from its own location ($(DEVKIT)/bin), so no --devkit.
$(GTL)/libdemo.so: $(HERE)/build/host/libdemo.so
	$(DEVKIT)/bin/LoreMakeThunk.py --name demo --lib $< --header demo.h -o $(HERE)/thunks -- -I$(SRC)

# 4. run main under the plugin, with the generated guest thunk on the guest LD_LIBRARY_PATH in place
# of its own libdemo.so, so the variadic and callback calls are carried across to the host. -L points
# qemu at the devkit's x86_64 sysroot for the guest loader/libc (needed on a non-x86_64 host, harmless
# on x86_64).
run: all
	LD_LIBRARY_PATH=$(DEVKIT)/lib:$(HERE)/build/host \
	  $(QEMU) -L $(DEVKIT)/x86_64/sysroot -plugin $(PLUGIN) \
	  -E LD_LIBRARY_PATH=$(DEVKIT)/x86_64/lib:$(GTL) \
	  $(HERE)/build/guest/main

clean:
	rm -rf $(HERE)/build $(HERE)/thunks
