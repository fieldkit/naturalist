#ifndef CHECK_NATURALIST_H_INCLUDED
#define CHECK_NATURALIST_H_INCLUDED

#include "check_core.h"
#include "board_definition.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_SHT31.h>

namespace fk {

class CheckNaturalist : public CheckCore {
private:
    TwoWireBus bno055Wire_{ Wire4and3 };
    Adafruit_BNO055 bnoSensor_{ 55, BNO055_ADDRESS_A, bno055Wire_.twoWire() };
    Adafruit_TSL2591 tsl2591Sensor_{ 2591 };
    Adafruit_MPL3115A2 mpl3115a2Sensor_;
    Adafruit_SHT31 sht31Sensor_;

public:
    bool sht31();
    bool mpl3115a2();
    bool tsl2591();
    bool bno055();
    bool sph0645();

public:
    bool check() override;
    void sample() override;

public:
    bool toggle_peripherals() override {
        return false;
    }

};

}

#endif
