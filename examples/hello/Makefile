# Build and run the hello thunk example. It shows the thunk as a drop-in: the same guest program
# prints "from guest" when its own library is on LD_LIBRARY_PATH, and "from host" once the generated
# guest thunk is put on LD_LIBRARY_PATH in its place.
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

# 1. the guest library (x86_64) and the guest program. main links the guest library but carries no
# rpath, so which libhello.so it loads is decided entirely by LD_LIBRARY_PATH at run time.
$(HERE)/build/guest/main: $(SRC)/hello_guest.c $(SRC)/main.c $(SRC)/hello.h
	mkdir -p $(HERE)/build/guest
	$(DEVKIT)/bin/x86_64-linux-gnu-clang -shared -fPIC $(SRC)/hello_guest.c -o $(HERE)/build/guest/libhello.so
	$(DEVKIT)/bin/x86_64-linux-gnu-clang $(SRC)/main.c -I$(SRC) -L$(HERE)/build/guest -lhello -o $@

# 2. the host library, native to this host, for the host thunk to dispatch to.
$(HERE)/build/host/libhello.so: $(SRC)/hello_host.c $(SRC)/hello.h
	mkdir -p $(HERE)/build/host
	cc -shared -fPIC $(SRC)/hello_host.c -o $@

# 3. the thunk, generated from the host library into a self-contained LORELEI_THUNK_PATH pack under
# thunks/. LoreMakeThunk finds its devkit from its own location ($(DEVKIT)/bin), so no --devkit.
$(GTL)/libhello.so: $(HERE)/build/host/libhello.so
	$(DEVKIT)/bin/LoreMakeThunk.py --name hello --lib $< --header hello.h -o $(HERE)/thunks -- -I$(SRC)

# 4. run the same program twice. First plain, with its own guest library on the guest LD_LIBRARY_PATH,
# so it prints "from guest". Then under the plugin, with the generated guest thunk on the guest
# LD_LIBRARY_PATH in its place, so the call reaches the host build and prints "from host". -L points
# qemu at the devkit's x86_64 sysroot for the guest loader/libc (needed on a non-x86_64 host, harmless
# on x86_64).
run: all
	@echo "== without the thunk =="
	$(QEMU) -L $(DEVKIT)/x86_64/sysroot \
	  -E LD_LIBRARY_PATH=$(HERE)/build/guest \
	  $(HERE)/build/guest/main
	@echo "== with the thunk =="
	LORELEI_THUNK_PATH=$(HERE)/thunks \
	LD_LIBRARY_PATH=$(DEVKIT)/lib:$(HERE)/build/host \
	  $(QEMU) -L $(DEVKIT)/x86_64/sysroot -plugin $(PLUGIN) \
	  -E LD_LIBRARY_PATH=$(GTL):$(DEVKIT)/x86_64/lib \
	  $(HERE)/build/guest/main

clean:
	rm -rf $(HERE)/build $(HERE)/thunks
