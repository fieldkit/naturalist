#include <../src/I2S.h>

#include "check_naturalist.h"

namespace fk {

constexpr const char LogName[] = "Check";

using Log = SimpleLog<LogName>;

bool CheckNaturalist::check() {
    auto success = true;

    Wire.begin();
    Wire4and3.begin();

    if (!mpl3115a2()) {
        success = false;
    }
    if (!tsl2591()) {
        success = false;
    }
    if (!sht31()) {
        success = false;
    }
    if (!sph0645()) {
        success = false;
    }

    #if defined(FK_ENABLE_BNO05)
    if (!bno055()) {
        success = false;
    }
    #endif

    if (!CheckCore::check()) {
        success = false;
    }

    return success;
}

bool CheckNaturalist::sht31() {
    if (!sht31Sensor_.begin()) {
        Log::info("SHT31 FAILED");
        return false;
    }

    auto shtTemperature = sht31Sensor_.readTemperature();
    Log::info("SHT31 %f", shtTemperature);
    Log::info("SHT31 PASSED");

    return true;
}

bool CheckNaturalist::mpl3115a2() {
    if (!mpl3115a2Sensor_.begin()) {
        Log::info("MPL3115A2 FAILED");
        return false;
    }

    auto pressurePascals = mpl3115a2Sensor_.getPressure();
    Log::info("MPL3115A2 %f", pressurePascals);
    Log::info("MPL3115A2 PASSED");

    return true;
}

bool CheckNaturalist::tsl2591() {
    if (!tsl2591Sensor_.begin()) {
        Log::info("TSL25911FN FAILED");
        return false;
    }

    Log::info("TSL25911FN PASSED");

    return true;
}

bool CheckNaturalist::bno055() {
    Log::info("BNO055 Checking...");

    if (!bnoSensor_.begin()) {
        Log::info("BNO055 FAILED");
        return false;
    }

    bnoSensor_.setExtCrystalUse(true);

    Log::info("BNO055 PASSED");

    return true;
}

bool CheckNaturalist::sph0645() {
    Log::info("SPH0645 Checking...");

    if (!I2S.begin(I2S_PHILIPS_MODE, 16000, 32)) {
        Log::info("SPH0645 FAILED");
        return false;
    }

    Log::info("SPH0645 PASSED");

    return true;
}

void CheckNaturalist::sample() {
    CheckCore::sample();

    if (enabled()) {
        auto shtTemperature = sht31Sensor_.readTemperature();
        auto shtHumidity = sht31Sensor_.readHumidity();

        Log::info("sensors: %fC %f%%", shtTemperature, shtHumidity);

        auto pressurePascals = mpl3115a2Sensor_.getPressure();
        auto altitudeMeters = mpl3115a2Sensor_.getAltitude();
        auto mplTempCelsius = mpl3115a2Sensor_.getTemperature();
        auto pressureInchesMercury = pressurePascals / 3377.0;

        Log::info("sensors: %fC %fpa %f\"/Hg %fm", mplTempCelsius, pressurePascals, pressureInchesMercury, altitudeMeters);

        auto fullLuminosity = tsl2591Sensor_.getFullLuminosity();
        auto ir = fullLuminosity >> 16;
        auto full = fullLuminosity & 0xFFFF;
        auto lux = tsl2591Sensor_.calculateLux(full, ir);

        Log::info("sensors: ir(%lu) full(%lu) visible(%lu) lux(%f)", ir, full, full - ir, lux);

        #if defined(FK_ENABLE_BNO05)
        uint8_t system = 0, gyro = 0, accel = 0, mag = 0;
        bnoSensor.getCalibration(&system, &gyro, &accel, &mag);

        sensors_event_t event;
        bnoSensor.getEvent(&event);

        Log::info("sensors: cal(%d, %d, %d, %d) xyz(%f, %f, %f)", system, gyro, accel, mag, event.orientation.x, event.orientation.y, event.orientation.z);
        #endif
    }
}

}
