file(GLOB SOURCE_FILES *.cpp)

set(LIBRARIES
  Wire
  SPI
  I2S
)

include(BuildInfo)
include(RandomDefine)

include_directories(./)

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
