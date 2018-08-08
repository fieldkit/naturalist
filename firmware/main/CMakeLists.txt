include(BuildInfo)
include(RandomSeed)
include(ArduinoMinimal)

include_directories(./)
include_directories(${RANDOM_SEED_INCLUDE})

add_definitions(-DFK_NATURALIST)
add_definitions(-DFK_CORE)

file(GLOB SOURCE_FILES *.cpp)
set(LIBRARIES Wire SPI I2S)

arduino(fk-naturalist-main "${SOURCE_FILES}" "${LIBRARIES}")

option(FK_ENABLE_RADIO "Enable/disable LoRa radio." OFF)
if(FK_ENABLE_RADIO)
  target_compile_definitions(arduino_zero_firmware-common PUBLIC -DFK_ENABLE_RADIO)
else()
  target_compile_definitions(arduino_zero_firmware-common PUBLIC -DFK_DISABLE_RADIO)
endif(FK_ENABLE_RADIO)

target_compile_definitions(arduino_zero_firmware-common PUBLIC -DFK_NATURALIST -DFK_CORE)

option(FK_DEBUG_INSTRUMENTATION "Enable function instrumentation." OFF)
if(FK_DEBUG_INSTRUMENTATION)
  target_compile_options(arduino_zero_Wire PUBLIC -finstrument-functions)
  target_compile_options(arduino_zero_phylum PUBLIC -finstrument-functions)
  target_compile_options(arduino_zero_WiFi101 PUBLIC -finstrument-functions)
  target_compile_options(fk-naturalist-main PUBLIC -finstrument-functions)
endif(FK_DEBUG_INSTRUMENTATION)
