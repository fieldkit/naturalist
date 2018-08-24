BUILD=build
SHELL := /bin/bash

default: all

$(BUILD):
	mkdir -p $(BUILD)

firmware/main/config.h:
	cp firmware/main/config.h.template firmware/main/config.h

all: cmake firmware/main/config.h gitdeps
	cd $(BUILD) && make

cmake: $(BUILD) gitdeps
	cd $(BUILD) && cmake ../

gitdeps:
	simple-deps --config firmware/test/dependencies.sd
	simple-deps --config firmware/main/dependencies.sd

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
