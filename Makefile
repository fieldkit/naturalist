BUILD ?= $(abspath build)
SHELL := /bin/bash

default: all

$(BUILD): firmware/test/config.h firmware/main/config.h
	mkdir -p $(BUILD)

firmware/test/config.h:
	cp firmware/test/config.h.template firmware/test/config.h

firmware/main/config.h:
	cp firmware/main/config.h.template firmware/main/config.h

all: cmake firmware/main/config.h gitdeps
	$(MAKE) -C $(BUILD)

cmake: $(BUILD) gitdeps
	cd $(BUILD) && cmake ../

gitdeps:
	simple-deps --config firmware/test/dependencies.sd
	simple-deps --config firmware/main/dependencies.sd

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
