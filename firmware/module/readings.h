#ifndef FK_WEATHER_READINGS_H_INCLUDED
#define FK_WEATHER_READINGS_H_INCLUDED

#include <fk-module.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_SHT31.h>

namespace fk {

class WeatherReadings : public Task {
private:
    TwoWire bno055Wire{ &sercom1, 11, 13 };
    Adafruit_SHT31 sht31Sensor;
    Adafruit_MPL3115A2 mpl3115a2Sensor;
    Adafruit_TSL2591 tsl2591Sensor{ 2591 };
    Adafruit_BNO055 bnoSensor{ 55, BNO055_ADDRESS_A, &bno055Wire };
    PendingSensorReading *pending;

public:
    WeatherReadings();

public:
    TaskEval task() override;

public:
    void setup();
    void begin(PendingSensorReading &p) {
        pending = &p;
        pending->elapsed = millis();
    }

};

};

#endif
