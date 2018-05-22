file(GLOB SOURCE_FILES *.cpp)

set(LIBRARIES
  Wire
  SPI
  I2S
)

include(BuildInfo)
include(RandomSeed)

include_directories(./)
include_directories(${RANDOM_SEED_INCLUDE})

set(ARDUINO_BOOTLOADER "${CMAKE_CURRENT_SOURCE_DIR}/flash_with_bootloader_and_noinit.ld")

add_definitions(-DFK_NATURALIST)
add_definitions(-DFK_CORE)

arduino(fk-naturalist-main "${SOURCE_FILES}" "${LIBRARIES}")

option(DISABLE_RADIO "Disable radio.")
if(DISABLE_RADIO)
else()
  target_compile_definitions(arduino_zero_firmware-common_src PUBLIC -DFK_ENABLE_RADIO)
  add_definitions(-DFK_ENABLE_RADIO)
endif(DISABLE_RADIO)

target_compile_definitions(arduino_zero_firmware-common_src PUBLIC -DFK_NATURALIST -DFK_CORE)

target_compile_options(arduino_zero_Wire PUBLIC -finstrument-functions)
target_compile_options(arduino_zero_fkfs PUBLIC -finstrument-functions)
target_compile_options(arduino_zero_WiFi101 PUBLIC -finstrument-functions)
target_compile_options(fk-naturalist-main PUBLIC -Wall -Werror -finstrument-functions)
