#include <Arduino.h>
#include <wiring_private.h>

#include "readings.h"

namespace fk {

WeatherReadings::WeatherReadings() : Task("Weather") {
}

void WeatherReadings::setup() {
    Wire.begin();

    bno055Wire.begin();

    pinPeripheral(11, PIO_SERCOM);
    pinPeripheral(13, PIO_SERCOM);

    if (!sht31Sensor.begin()) {
        debugfln("SHT31 FAILED");
    } 
    if (!mpl3115a2Sensor.begin()) {
        debugfln("MPL3115A2 FAILED");
    }

    if (!tsl2591Sensor.begin()) {
        debugfln("TSL25911FN FAILED");
    }

    if (!bnoSensor.begin()) {
        debugfln("BNO055 FAILED");
    } else {
        bnoSensor.setExtCrystalUse(true);
    }
}

TaskEval WeatherReadings::task() {
    Wire.begin();

    auto shtTemperature = sht31Sensor.readTemperature();
    auto shtHumidity = sht31Sensor.readHumidity();

    auto pressurePascals = mpl3115a2Sensor.getPressure();
    auto altitudeMeters = mpl3115a2Sensor.getAltitude();
    auto mplTempCelsius = mpl3115a2Sensor.getTemperature();
    auto pressureInchesMercury = pressurePascals / 3377.0;

    auto fullLuminosity = tsl2591Sensor.getFullLuminosity();
    auto ir = fullLuminosity >> 16;
    auto full = fullLuminosity & 0xFFFF;
    auto lux = tsl2591Sensor.calculateLux(full, ir);

    uint8_t system = 0, gyro = 0, accel = 0, mag = 0;
    bnoSensor.getCalibration(&system, &gyro, &accel, &mag);

    sensors_event_t event;
    bnoSensor.getEvent(&event);

    if (pending != nullptr) {
        auto readings = pending->readings;
        readings[ 0].value = shtTemperature;
        readings[ 1].value = shtHumidity;
        readings[ 2].value = mplTempCelsius;
        readings[ 3].value = pressurePascals;
        readings[ 4].value = altitudeMeters;
        readings[ 5].value = ir;
        readings[ 6].value = full - ir;
        readings[ 7].value = lux;
        readings[ 8].value = system;
        readings[ 9].value = event.orientation.x;
        readings[10].value = event.orientation.y;
        readings[11].value = event.orientation.z;

        pending->elapsed -= millis();

        for (auto i = 0; i < 12; ++i) {
            readings[i].status = SensorReadingStatus::Done;
            readings[i].time = clock.getTime();
        }
    }

    log("Sensors: %fC %f%%, %fC %fpa %f\"/Hg %fm", shtTemperature, shtHumidity, mplTempCelsius, pressurePascals, pressureInchesMercury, altitudeMeters);
    log("Sensors: ir(%d) full(%d) visible(%d) lux(%d)", ir, full, full - ir, lux);
    log("Sensors: cal(%d, %d, %d, %d) xyz(%f, %f, %f)", system, gyro, accel, mag, event.orientation.x, event.orientation.y, event.orientation.z);

    return TaskEval::done();
}

}
