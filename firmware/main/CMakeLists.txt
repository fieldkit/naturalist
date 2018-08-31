include(Arduino)
include(BuildInfo)
include(RandomSeed)

enable_arduino_toolchain()

file(GLOB sources *.cpp)

add_arduino_library(fk-naturalist-main "${sources}")

add_definitions(-DFK_NATURALIST)

find_package(FkCore)

fk_configure_core(fk-naturalist-main)

target_include_directories(fk-naturalist-main PRIVATE ./)

target_include_directories(fk-naturalist-main PRIVATE ${RANDOM_SEED_INCLUDE})

fk_core_apply_configuration(fk-naturalist-main)

target_compile_options(firmware-common-fk-naturalist-main PUBLIC -DFK_NATURALIST)

find_package(ArduinoSound)
target_link_libraries(fk-naturalist-main ArduinoSound)
target_link_libraries(firmware-common-fk-naturalist-main ArduinoSound)

find_package(I2S)
target_link_libraries(fk-naturalist-main I2S)

find_package(Adafruit_Sensor)
target_link_libraries(fk-naturalist-main Adafruit_Sensor)

find_package(Adafruit_BNO055)
target_link_libraries(fk-naturalist-main Adafruit_BNO055)

find_package(Adafruit_TSL2561)
target_link_libraries(fk-naturalist-main Adafruit_TSL2561)

find_package(Adafruit_MPL3115A2)
target_link_libraries(fk-naturalist-main Adafruit_MPL3115A2_Library)

find_package(Adafruit_SHT31)
target_link_libraries(fk-naturalist-main Adafruit_SHT31)

find_package(Adafruit_TSL2591)
target_link_libraries(fk-naturalist-main Adafruit_TSL2591_Library)

fk_write_profile(fk-naturalist-main)

add_arduino_firmware(fk-naturalist-main)
