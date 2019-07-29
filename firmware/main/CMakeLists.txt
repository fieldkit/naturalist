include(Arduino)
include(BuildInfo)
include(RandomSeed)
include(./dependencies.cmake)

enable_arduino_toolchain()

file(GLOB sources *.cpp ../common/*.cpp)

add_arduino_library(fk-naturalist-standard "${sources}")

target_include_directories(fk-naturalist-standard PRIVATE "../common")

target_include_directories(fk-naturalist-standard PRIVATE ${RANDOM_SEED_INCLUDE})

add_definitions(-DFK_NATURALIST)

find_package(FkCore)

fk_configure_core(fk-naturalist-standard)

fk_core_apply_configuration(fk-naturalist-standard)

target_compile_options(firmware-common-fk-naturalist-standard PUBLIC -DFK_NATURALIST)

target_compile_options(firmware-common-fk-naturalist-standard PUBLIC -DFK_WIFI_STARTUP_ONLY)

# target_compile_options(phylum PUBLIC -DPHYLUM_DEBUG=10)
# target_compile_options(firmware-common-fk-naturalist-standard PUBLIC -DPHYLUM_DEBUG=10)

find_package(ArduinoSound)
target_link_libraries(fk-naturalist-standard ArduinoSound)
target_link_libraries(firmware-common-fk-naturalist-standard ArduinoSound)

find_package(I2S)
target_link_libraries(fk-naturalist-standard I2S)

find_package(Adafruit_Sensor)
target_link_libraries(fk-naturalist-standard Adafruit_Sensor)

find_package(Adafruit_BNO055)
target_link_libraries(fk-naturalist-standard Adafruit_BNO055)

find_package(Adafruit_TSL2561)
target_link_libraries(fk-naturalist-standard Adafruit_TSL2561)

find_package(Adafruit_MPL3115A2)
target_link_libraries(fk-naturalist-standard Adafruit_MPL3115A2_Library)

find_package(Adafruit_SHT31)
target_link_libraries(fk-naturalist-standard Adafruit_SHT31)

find_package(Adafruit_TSL2591)
target_link_libraries(fk-naturalist-standard Adafruit_TSL2591_Library)

fk_write_profile(fk-naturalist-standard)

add_arduino_firmware(fk-naturalist-standard)
