include(ArduinoMinimal)

include_directories(./)

set(SOURCE_FILES main.cpp debug.cpp)
set(LIBRARIES Wire SPI SD I2S)

arduino(fk-naturalist-test "${SOURCE_FILES}" "${LIBRARIES}")

add_definitions(-DPHYLUM_ENABLE_SERIAL_FLASH -DPHYLUM_ENABLE_SD)
target_compile_options(arduino_zero_phylum PUBLIC -DPHYLUM_ENABLE_SERIAL_FLASH -DPHYLUM_ENABLE_SD)
