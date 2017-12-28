set(SOURCE_FILES
  main.cpp
  debug.cpp
  AnalogSampling.cpp
)

set(LIBRARIES
  SPI
  Wire
)

include_directories(./)

arduino(fk-naturalist-test "${SOURCE_FILES}" "${LIBRARIES}")
