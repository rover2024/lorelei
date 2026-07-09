# Build and run the hello thunk example. It shows the thunk as a drop-in: the same guest program
# prints "from guest" with its own library, and "from host" once the generated guest thunk is swapped
# in ahead of it via LD_LIBRARY_PATH.
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
GTL    := $(HERE)/thunks/x86_64/lib/x86_64-LoreGTL

.PHONY: all run clean

all: $(HERE)/build/guest/main $(GTL)/libhello.so

# 1. the guest library (x86_64) and the guest program. main links the guest library with an rpath of
# $ORIGIN, so on its own it loads that build/guest/libhello.so sitting next to it.
$(HERE)/build/guest/main: $(SRC)/hello_guest.c $(SRC)/main.c $(SRC)/hello.h
	mkdir -p $(HERE)/build/guest
	$(DEVKIT)/bin/x86_64-linux-gnu-clang -shared -fPIC -o $(HERE)/build/guest/libhello.so $(SRC)/hello_guest.c
	$(DEVKIT)/bin/x86_64-linux-gnu-clang $(SRC)/main.c -I$(SRC) -L$(HERE)/build/guest -lhello \
	    -Wl,-rpath,'$$ORIGIN' -o $@

# 2. the host library, native to this host, for the host thunk to dispatch to.
$(HERE)/build/host/libhello.so: $(SRC)/hello_host.c $(SRC)/hello.h
	mkdir -p $(HERE)/build/host
	cc -shared -fPIC -o $@ $(SRC)/hello_host.c

# 3. the thunk, generated from the host library into a self-contained LORELEI_THUNK_PATH pack under
# thunks/. LoreMakeThunk finds its devkit from its own location ($(DEVKIT)/bin), so no --devkit.
$(GTL)/libhello.so: $(HERE)/build/host/libhello.so
	$(DEVKIT)/bin/LoreMakeThunk.py --name hello --lib $< --header hello.h -o $(HERE)/thunks -- -I$(SRC)

# 4. run the same program twice. First plain: it loads its own guest library and prints "from guest".
# Then under the plugin, with the generated guest thunk ahead of it on LD_LIBRARY_PATH, so the call
# reaches the host build and prints "from host". -L points qemu at the devkit's x86_64 sysroot for the
# guest loader/libc (needed on a non-x86_64 host, harmless on x86_64).
run: all
	@echo "== without the thunk =="
	$(QEMU) -L $(DEVKIT)/x86_64/sysroot $(HERE)/build/guest/main
	@echo "== with the thunk =="
	LORELEI_THUNK_PATH=$(HERE)/thunks \
	LD_LIBRARY_PATH=$(DEVKIT)/lib:$(HERE)/build/host \
	  $(QEMU) -L $(DEVKIT)/x86_64/sysroot -plugin $(PLUGIN) \
	  -E LD_LIBRARY_PATH=$(GTL):$(DEVKIT)/x86_64/lib \
	  $(HERE)/build/guest/main

clean:
	rm -rf $(HERE)/build $(HERE)/thunks
