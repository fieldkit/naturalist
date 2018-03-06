set(SOURCE_FILES
  main.cpp
  debug.cpp
  AnalogSampling.cpp
)

set(LIBRARIES
  Wire
  SPI
  SD
)

include_directories(./)

arduino(fk-naturalist-test "${SOURCE_FILES}" "${LIBRARIES}")
