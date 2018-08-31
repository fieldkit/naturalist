include(Arduino)

enable_arduino_toolchain()

include_directories(./)

set(sources main.cpp)

add_arduino_library(fk-naturalist-test "${sources}")

add_definitions(-DFK_NATURALIST)

find_package(FkCore)

find_package(phylum)
target_link_libraries(fk-naturalist-test phylum)

fk_configure_core(fk-naturalist-test)

target_compile_options(fk-naturalist-test PRIVATE -Wall -Werror)

fk_core_apply_configuration(fk-naturalist-test)

find_package(ArduinoSound)
target_link_libraries(fk-naturalist-test ArduinoSound)
target_link_libraries(firmware-common-fk-naturalist-test ArduinoSound)

find_package(I2S)
target_link_libraries(fk-naturalist-test I2S)

find_package(Adafruit_Sensor)
target_link_libraries(fk-naturalist-test Adafruit_Sensor)

find_package(Adafruit_BNO055)
target_link_libraries(fk-naturalist-test Adafruit_BNO055)

find_package(Adafruit_TSL2561)
target_link_libraries(fk-naturalist-test Adafruit_TSL2561)

find_package(Adafruit_MPL3115A2)
target_link_libraries(fk-naturalist-test Adafruit_SHT31)

find_package(Adafruit_SHT31)
target_link_libraries(fk-naturalist-test Adafruit_TSL2591_Library)

find_package(Adafruit_TSL2591)
target_link_libraries(fk-naturalist-test Adafruit_MPL3115A2_Library)

add_arduino_firmware(fk-naturalist-test)
