.DEFAULT_GOAL := test
HOST_CC ?= cc
HOST_CFLAGS ?= -O2 -g -std=gnu89
CROSS_CC ?= m68k-cbm-sysv4-gcc
BUILD := $(CURDIR)/build

.PHONY: test sanitize module simulation userland check-sources

test: $(BUILD)/test-hw
	$(BUILD)/test-hw

$(BUILD)/test-hw: src/test-hw.c src/hw.c src/hw.h
	mkdir -p $(BUILD)
	$(HOST_CC) $(HOST_CFLAGS) -o $@ src/test-hw.c

sanitize: $(BUILD)/test-hw-san
	ASAN_OPTIONS=detect_leaks=1 $(BUILD)/test-hw-san

$(BUILD)/test-hw-san: src/test-hw.c src/hw.c src/hw.h
	mkdir -p $(BUILD)
	$(HOST_CC) -O1 -g -std=gnu89 -fno-omit-frame-pointer -fsanitize=address,undefined -o $@ src/test-hw.c

module:
	$(MAKE) -C src KIT="$(KIT)" MODOUT="$(BUILD)/production"

simulation:
	$(MAKE) -C src KIT="$(KIT)" MODOUT="$(BUILD)/simulation" CFLAGS=-DLANIDE_NATIVE_TEST

userland:
	mkdir -p $(BUILD)
	$(CROSS_CC) -O -traditional -m68040 -static -o $(BUILD)/lanctl src/lanctl.c

check-sources:
	sha256sum -c evidence/stable-sources.sha256
