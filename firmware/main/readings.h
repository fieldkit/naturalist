#ifndef FK_NATURALIST_READINGS_H_INCLUDED
#define FK_NATURALIST_READINGS_H_INCLUDED

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_SHT31.h>

#include <AudioAnalyzer.h>
#include <AudioIn.h>
#include <AmplitudeAnalyzer.h>
#include <AudioInI2S.h>

#include "state_services.h"

#include "task.h"
#include "core_state.h"
#include "two_wire.h"

namespace fk {

class NaturalistReadings {
private:
    TwoWireBus bno055Wire{ Wire4and3 };
    Adafruit_SHT31 sht31Sensor;
    Adafruit_MPL3115A2 mpl3115a2Sensor;
    Adafruit_TSL2591 tsl2591Sensor{ 2591 };
    Adafruit_BNO055 bnoSensor{ 55, BNO055_ADDRESS_A, &Wire4and3 };
    AmplitudeAnalyzer amplitudeAnalyzer;
    bool hasBno055{ false };
    bool hasAudioAnalyzer{ false };
    bool initialized{ false };

public:
    void setup();
    TaskEval task(CoreState &state);

};

class TakeNaturalistReadings : public MainServicesState {
private:
    NaturalistReadings readings_;

public:
    const char *name() const override {
        return "TakeNaturalistReadings";
    }

public:
    void task() override;

};

}

#endif
