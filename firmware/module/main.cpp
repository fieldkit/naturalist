#include <Arduino.h>
#include <Wire.h>

#include <fk-module.h>

#include "readings.h"

namespace fk {

class WeatherModule : public Module {
private:
    Delay delay{ 500 };
    WeatherReadings *weatherReadings;

public:
    WeatherModule(ModuleInfo &info, WeatherReadings &weatherReadings);

public:
    ModuleReadingStatus beginReading(SensorReading *readings) override;
    ModuleReadingStatus readingStatus(SensorReading *readings) override;
    void done(Task &task) override;

};

WeatherModule::WeatherModule(ModuleInfo &info, WeatherReadings &weatherReadings) :
    Module(info), weatherReadings(&weatherReadings) {
}

ModuleReadingStatus WeatherModule::beginReading(SensorReading *readings) {
    weatherReadings->begin(readings);
    push(delay); // This is to give us time to reply with the backoff. Should be avoidable?
    push(*weatherReadings);

    return ModuleReadingStatus{ 5000 };
}

ModuleReadingStatus WeatherModule::readingStatus(SensorReading *readings) {
    return ModuleReadingStatus{};
}

void WeatherModule::done(Task &task) {
    resume();
}

}

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial && millis() < 2000) {
        delay(100);
    }

    debugfpln("Module", "Starting (%lu free)", fk_free_memory());

    fk::ModuleInfo info = {
        8,
        12,
        "Weather",
        {
            {"Temp #1", "°C",},
            {"Humidity", "%",},
            {"Temp #2", "°C",},
            {"Pressure", "pa",},
            {"Altitude", "m",},
            {"Ambient IR", "",},
            {"Ambient Visible", "",},
            {"Ambient Lux", "",},
            {"IMU Cal", ""},
            {"IMU Orien X", ""},
            {"IMU Orien Y", ""},
            {"IMU Orien Z", ""},
        },
        {
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
        },
    };

    fk::WeatherReadings weatherReadings;
    fk::WeatherModule module(info, weatherReadings);
    uint32_t idleStart = 0;

    weatherReadings.setup();

    module.begin();

    while (true) {
        module.tick();

        delay(10);
    }
}

void loop() {
}

}
