set(SOURCE_FILES
  main.cpp
  debug.cpp
)

set(LIBRARIES
  Wire
  SPI
  SD
  I2S
)

include_directories(./)

arduino(fk-naturalist-test "${SOURCE_FILES}" "${LIBRARIES}")
