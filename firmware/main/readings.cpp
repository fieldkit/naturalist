#include <math.h>

#include "readings.h"

namespace fk {

void NaturalistReadings::setup() {
    if (!amplitudeAnalyzer.input(AudioInI2S)) {
        log("Amplitude Analyzer failed");
    }
    else {
        log("Amplitude Analyzer ready.");
        hasAudioAnalyzer = true;
    }

    Wire.begin();

    if (!sht31Sensor.begin()) {
        log("SHT31 FAILED");
    }
    if (!mpl3115a2Sensor.begin()) {
        log("MPL3115A2 FAILED");
    }

    if (!tsl2591Sensor.begin()) {
        log("TSL25911FN FAILED");
    }

    if (!bno055Wire.begin()) {
        log("BNO055 FAILED");
    }
    else {
        if (!bnoSensor.begin()) {
            log("BNO055 FAILED");
        } else {
            hasBno055 = true;
            bnoSensor.setExtCrystalUse(true);
        }
    }
}

void NaturalistReadings::enqueued() {
}

TaskEval NaturalistReadings::task() {
    constexpr uint32_t AudioSamplingDuration = 2000;

    log("Ready, listening...");

    auto numberOfSamples = 0;
    auto audioRmsMin = 0.0f;
    auto audioRmsMax = 0.0f;
    auto total = 0.0f;
    auto start = millis();
    while (millis() - start < AudioSamplingDuration) {
        if (amplitudeAnalyzer.available()) {
            auto amplitude = amplitudeAnalyzer.read();
            if (amplitude > 0) {
                if (numberOfSamples == 0) {
                    audioRmsMin = amplitude;
                    audioRmsMax = amplitude;
                }
                else {
                    if (audioRmsMax < amplitude) {
                        audioRmsMax = amplitude;
                    }
                    if (audioRmsMin > amplitude) {
                        audioRmsMin = amplitude;
                    }
                }
                total += amplitude;
                numberOfSamples++;
            }
        }
    }

    auto audioRmsAvg = total / (float)numberOfSamples;
    auto audioDbfsAvg = 20.0f * log10(audioRmsAvg);
    auto audioDbfsMin = 20.0f * log10(audioRmsMin);
    auto audioDbfsMax = 20.0f * log10(audioRmsMax);

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
    sensors_event_t event;
    memzero(&event, sizeof(sensors_event_t));
    if (hasBno055) {
        bnoSensor.getCalibration(&system, &gyro, &accel, &mag);
        bnoSensor.getEvent(&event);
    }

    float values[] = {
        shtTemperature,
        shtHumidity,
        mplTempCelsius,
        pressurePascals,
        altitudeMeters,
        (float)ir,
        (float)full - ir,
        lux,
        (float)system,
        event.orientation.x,
        event.orientation.y,
        event.orientation.z,
        audioRmsAvg,
        audioRmsMin,
        audioRmsMax,
        (float)audioDbfsAvg,
        (float)audioDbfsMin,
        (float)audioDbfsMax
    };
    auto time = clock.getTime();
    for (size_t i = 0; i < sizeof(values) / sizeof(float); ++i) {
        IncomingSensorReading reading{
            (uint8_t)i,
            time,
            values[i],
        };
        state->merge(0, reading);
    }

    log("Sensors: %fC %f%%, %fC %fpa %f\"/Hg %fm", shtTemperature, shtHumidity, mplTempCelsius, pressurePascals, pressureInchesMercury, altitudeMeters);
    log("Sensors: ir(%lu) full(%lu) visible(%lu) lux(%f)", ir, full, full - ir, lux);
    log("Sensors: cal(%d, %d, %d, %d) xyz(%f, %f, %f)", system, gyro, accel, mag, event.orientation.x, event.orientation.y, event.orientation.z);
    log("Sensors: RMS: min=%f max=%f avg=%f range=%f (%d samples)", audioRmsMin, audioRmsMax, audioRmsMax - audioRmsMin, audioRmsAvg, numberOfSamples);
    log("Sensors: dbfs: min=%f max=%f avg=%f", audioDbfsMin, audioDbfsMax, audioDbfsAvg);

    return TaskEval::done();
}

}
