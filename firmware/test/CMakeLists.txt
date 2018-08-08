include(ArduinoMinimal)

include_directories(./)

set(SOURCE_FILES main.cpp debug.cpp)
set(LIBRARIES Wire SPI SD I2S)

arduino(fk-naturalist-test "${SOURCE_FILES}" "${LIBRARIES}")
