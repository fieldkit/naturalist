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

#include "active_object.h"
#include "core_state.h"
#include "two_wire.h"

namespace fk {

class NaturalistReadings : public Task {
private:
    CoreState *state;
    TwoWireBus bno055Wire{ Wire4and3 };
    Adafruit_SHT31 sht31Sensor;
    Adafruit_MPL3115A2 mpl3115a2Sensor;
    Adafruit_TSL2591 tsl2591Sensor{ 2591 };
    Adafruit_BNO055 bnoSensor{ 55, BNO055_ADDRESS_A, &Wire4and3 };
    AmplitudeAnalyzer amplitudeAnalyzer;
    bool hasBno055{ false };
    bool hasAudioAnalyzer{ false };

public:
    NaturalistReadings(CoreState &state) : Task("Naturalist"), state(&state) {
    }

public:
    void setup();
    void enqueued() override;
    TaskEval task() override;

};

}

#endif
