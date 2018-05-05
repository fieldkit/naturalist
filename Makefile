BUILD=build
SHELL := /bin/bash

default: all

$(BUILD):
	mkdir -p $(BUILD)

firmware/main/config.h:
	cp firmware/main/config.h.template firmware/main/config.h

all: cmake firmware/main/config.h gitdeps seed
	cd $(BUILD) && make

cmake: $(BUILD) gitdeps
	cd $(BUILD) && cmake ../

seed:
	echo "// Generated before compile time to seed the RNG." > firmware/main/seed.h
	echo "" >> firmware/main/seed.h
	echo "#define RANDOM_SEED $$RANDOM" >> firmware/main/seed.h

gitdeps:
	simple-deps --config firmware/test/arduino-libraries
	simple-deps --config firmware/module/arduino-libraries
	simple-deps --config firmware/main/arduino-libraries

clean:
	rm -rf $(BUILD)

veryclean: clean
	rm -rf gitdeps
