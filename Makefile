BUILD=build
SHELL := /bin/bash
BUILD_TAG ?= $(shell hostname)

default: all

$(BUILD):
	mkdir -p $(BUILD)

firmware/main/config.h:
	cp firmware/main/config.h.template firmware/main/config.h

all: $(BUILD) firmware/main/config.h gitdeps seed
	cd $(BUILD) && cmake ../
	cd $(BUILD) && make

seed: GIT_HASH=$(shell git log -1 --pretty=format:"%H")
seed:
	echo "// Generated before compile time to seed the RNG." > firmware/main/seed.h
	echo "" >> firmware/main/seed.h
	echo "#define RANDOM_SEED $$RANDOM" >> firmware/main/seed.h
	echo "#define FIRMWARE_GIT_HASH \"$(GIT_HASH)\"" >> firmware/main/seed.h
	echo "#define FIRMWARE_BUILD \"$(BUILD_TAG)\"" >> firmware/main/seed.h

gitdeps:
	simple-deps --config firmware/test/arduino-libraries
	simple-deps --config firmware/module/arduino-libraries
	simple-deps --config firmware/main/arduino-libraries

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
