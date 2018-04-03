#include <Arduino.h>
#include <Wire.h>

#include <fk-module.h>

#include "readings.h"

namespace fk {

class WeatherModule : public Module {
private:
    TwoWireBus bus{ Wire };
    Delay delay{ 500 };
    NaturalistReadings *readings;

public:
    WeatherModule(ModuleInfo &info, NaturalistReadings &readings);

public:
    ModuleReadingStatus beginReading(PendingSensorReading &pending) override;
    ModuleReadingStatus readingStatus(PendingSensorReading &pending) override;
    void done(Task &task) override;

};

WeatherModule::WeatherModule(ModuleInfo &info, NaturalistReadings &readings) :
    Module(bus, info), readings(&readings) {
}

ModuleReadingStatus WeatherModule::beginReading(PendingSensorReading &pending) {
    readings->begin(pending);
    push(delay); // This is to give us time to reply with the backoff. Should be avoidable?
    push(*readings);

    return ModuleReadingStatus{ 5000 };
}

ModuleReadingStatus WeatherModule::readingStatus(PendingSensorReading &pending) {
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
        fk_module_ModuleType_SENSOR,
        8,
        12,
        "FkNat",
        {
            {"temp_1", "°C",},
            {"humidity", "%",},
            {"temp_2", "°C",},
            {"pressure", "pa",},
            {"altitude", "m",},
            {"light_ir", "",},
            {"light_visible", "",},
            {"light_lux", "",},
            {"imu_cal", ""},
            {"imu_orien_x", ""},
            {"imu_orien_y", ""},
            {"imu_orien_z", ""},
        },
        {
            {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
        },
    };

    fk::NaturalistReadings readings;
    fk::WeatherModule module(info, readings);
    uint32_t idleStart = 0;

    readings.setup();

    module.begin();

    while (true) {
        module.tick();

        delay(10);
    }
}

void loop() {
}

}
