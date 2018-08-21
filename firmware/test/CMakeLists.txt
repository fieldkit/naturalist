include(Arduino)

enable_arduino_toolchain()

include_directories(./)

set(sources main.cpp debug.cpp)

add_arduino_library(fk-naturalist-test "${sources}")

find_package(FkCore)
find_package(FkCore)
find_package(I2S)
find_package(Adafruit_Sensor)
find_package(Adafruit_BNO055)
find_package(Adafruit_TSL2561)
find_package(Adafruit_TSL2591)
find_package(Adafruit_MPL3115A2)
find_package(Adafruit_SHT31)

target_compile_options(fk-naturalist-test PRIVATE -Wall -Werror)

target_link_libraries(fk-naturalist-test I2S)
target_link_libraries(fk-naturalist-test Adafruit_Sensor)
target_link_libraries(fk-naturalist-test Adafruit_BNO055)
target_link_libraries(fk-naturalist-test Adafruit_TSL2561)
target_link_libraries(fk-naturalist-test Adafruit_MPL3115A2_Library)
target_link_libraries(fk-naturalist-test Adafruit_SHT31)
target_link_libraries(fk-naturalist-test Adafruit_TSL2591_Library)
target_link_libraries(fk-naturalist-test firmware-common)
target_link_libraries(fk-naturalist-test SerialFlash)
target_link_libraries(fk-naturalist-test Wire)
target_link_libraries(fk-naturalist-test SPI)
target_link_libraries(fk-naturalist-test WiFi101)
target_link_libraries(fk-naturalist-test FuelGauge)
target_link_libraries(fk-naturalist-test phylum)
target_link_libraries(fk-naturalist-test arduino-logging)
target_link_libraries(fk-naturalist-test RTClib)

add_arduino_firmware(fk-naturalist-test)
