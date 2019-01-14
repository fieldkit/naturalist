#include <../src/I2S.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_SHT31.h>

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
    Adafruit_SHT31 sht31Sensor;

    if (!sht31Sensor.begin()) {
        Log::info("SHT31 FAILED");
        return false;
    }

    auto shtTemperature = sht31Sensor.readTemperature();
    Log::info("SHT31 %f", shtTemperature);
    Log::info("SHT31 PASSED");

    return true;
}

bool CheckNaturalist::mpl3115a2() {
    Adafruit_MPL3115A2 mpl3115a2Sensor;

    if (!mpl3115a2Sensor.begin()) {
        Log::info("MPL3115A2 FAILED");
        return false;
    }

    auto pressurePascals = mpl3115a2Sensor.getPressure();
    Log::info("MPL3115A2 %f", pressurePascals);
    Log::info("MPL3115A2 PASSED");

    return true;
}

bool CheckNaturalist::tsl2591() {
    Adafruit_TSL2591 tsl2591Sensor{ 2591 };

    if (!tsl2591Sensor.begin()) {
        Log::info("TSL25911FN FAILED");
        return false;
    }

    Log::info("TSL25911FN PASSED");

    return true;
}

bool CheckNaturalist::bno055() {
    Log::info("BNO055 Checking...");

    TwoWireBus bno055Wire{ Wire4and3 };
    Adafruit_BNO055 bnoSensor{ 55, BNO055_ADDRESS_A, bno055Wire.twoWire() };

    if (!bnoSensor.begin()) {
        Log::info("BNO055 FAILED");
        return false;
    }

    bnoSensor.setExtCrystalUse(true);

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

}
