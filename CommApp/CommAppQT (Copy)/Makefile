# ─────────────────────────────────────────────────────────────────────────────
# Makefile  –  IoT QEMU Client Application
# ─────────────────────────────────────────────────────────────────────────────
#
# Native build:
#   make
#
# Cross-compile with Yocto SDK (after sourcing the SDK env script):
#   make CXX=aarch64-poky-linux-g++ SYSROOT=<path-to-sysroot>
#
# Install to QEMU rootfs (adjust DESTDIR):
#   make install DESTDIR=/tmp/qemu-rootfs
# ─────────────────────────────────────────────────────────────────────────────

CXX      ?= g++
CXXFLAGS  = -std=c++17 -Wall -Wextra -O2
LDFLAGS   =

TARGET    = iot_client
SRCS      = client_main.cpp
HEADERS   = Channel.h Socket.h

PREFIX    ?= /usr/local
BINDIR     = $(PREFIX)/bin

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SRCS)
	@echo ""
	@echo "  Build successful: ./$(TARGET)"
	@echo "  Run with:  ./$(TARGET) [server_ip]"
	@echo ""

install: $(TARGET)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET)
