#include <math.h>

#include "readings.h"

namespace fk {

constexpr const char Log[] = "Naturalist";

using Logger = SimpleLog<Log>;

void TakeNaturalistReadings::task() {
    readings_.setup();

    while (is_task_running(readings_.task(*services().state))) {
        services().alive();
    }

    back();
}

void NaturalistReadings::setup() {
    if (initialized_) {
        return;
    }

    initialized_ = true;

    if (!amplitudeAnalyzer_.input(AudioInI2S)) {
        Logger::info("Amplitude Analyzer failed");
    }
    else {
        Logger::info("Amplitude Analyzer ready.");
        hasAudioAnalyzer_ = true;
    }

    Wire.begin();

    if (!sht31Sensor_.begin()) {
        Logger::info("SHT31 FAILED");
    }

    if (!mpl3115a2Sensor_.begin()) {
        Logger::info("MPL3115A2 FAILED");
    }

    if (!tsl2591Sensor_.begin()) {
        Logger::info("TSL25911FN FAILED");
    }

    if (!bno055Wire_.begin()) {
        Logger::info("BNO055 FAILED");
    }
    else {
        if (!bnoSensor_.begin()) {
            Logger::info("BNO055 FAILED");
        } else {
            hasBno055_ = true;
            bnoSensor_.setExtCrystalUse(true);
        }
    }
}

TaskEval NaturalistReadings::task(CoreState &state) {
    constexpr uint32_t AudioSamplingDuration = 2000;

    auto numberOfSamples = 0;
    auto audioRmsMin = 0.0f;
    auto audioRmsMax = 0.0f;
    auto total = 0.0f;
    if (hasAudioAnalyzer_) {
        auto start = fk_uptime();

        Logger::info("Ready, listening...");

        while (fk_uptime() - start < AudioSamplingDuration) {
            if (amplitudeAnalyzer_.available()) {
                auto amplitude = amplitudeAnalyzer_.read();
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
    }

    auto audioRmsAvg = numberOfSamples > 0 ? total / (float)numberOfSamples : 0.0f;
    auto audioDbfsAvg = numberOfSamples > 0 ? 20.0f * log10(audioRmsAvg) : 0.0f;
    auto audioDbfsMin = numberOfSamples > 0 ? 20.0f * log10(audioRmsMin) : 0.0f;
    auto audioDbfsMax = numberOfSamples > 0 ? 20.0f * log10(audioRmsMax) : 0.0f;

    auto shtTemperature = sht31Sensor_.readTemperature();
    auto shtHumidity = sht31Sensor_.readHumidity();

    auto pressurePascals = mpl3115a2Sensor_.getPressure();
    auto altitudeMeters = mpl3115a2Sensor_.getAltitude();
    auto mplTempCelsius = mpl3115a2Sensor_.getTemperature();
    auto pressureInchesMercury = pressurePascals / 3377.0;

    auto fullLuminosity = tsl2591Sensor_.getFullLuminosity();
    auto ir = fullLuminosity >> 16;
    auto full = fullLuminosity & 0xFFFF;
    auto lux = tsl2591Sensor_.calculateLux(full, ir);

    uint8_t system = 0, gyro = 0, accel = 0, mag = 0;
    sensors_event_t event;
    memset(&event, 0, sizeof(sensors_event_t));
    if (hasBno055_) {
        bnoSensor_.getCalibration(&system, &gyro, &accel, &mag);
        bnoSensor_.getEvent(&event);
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
    auto module = state.getModule(8);
    for (size_t i = 0; i < sizeof(values) / sizeof(float); ++i) {
        IncomingSensorReading reading{
            (uint8_t)i,
            time,
            values[i],
        };
        state.merge(*module, reading);
    }

    Logger::info("Sensors: %fC %f%%, %fC %fpa %f\"/Hg %fm", shtTemperature, shtHumidity, mplTempCelsius, pressurePascals, pressureInchesMercury, altitudeMeters);
    Logger::info("Sensors: ir(%lu) full(%lu) visible(%lu) lux(%f)", ir, full, full - ir, lux);
    Logger::info("Sensors: cal(%d, %d, %d, %d) xyz(%f, %f, %f)", system, gyro, accel, mag, event.orientation.x, event.orientation.y, event.orientation.z);
    Logger::info("Sensors: RMS: min=%f max=%f avg=%f range=%f (%d samples)", audioRmsMin, audioRmsMax, audioRmsMax - audioRmsMin, audioRmsAvg, numberOfSamples);
    Logger::info("Sensors: dbfs: min=%f max=%f avg=%f", audioDbfsMin, audioDbfsMax, audioDbfsAvg);

    return TaskEval::done();
}

}
