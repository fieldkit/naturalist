#include "naturalist_core_module.h"

namespace fk {

void NaturalistCoreModule::begin() {
    pinMode(Hardware::SD_PIN_CS, OUTPUT);
    pinMode(Hardware::WIFI_PIN_CS, OUTPUT);
    pinMode(Hardware::RFM95_PIN_CS, OUTPUT);
    pinMode(Hardware::FLASH_PIN_CS, OUTPUT);

    digitalWrite(Hardware::SD_PIN_CS, HIGH);
    digitalWrite(Hardware::WIFI_PIN_CS, HIGH);
    digitalWrite(Hardware::RFM95_PIN_CS, HIGH);
    digitalWrite(Hardware::FLASH_PIN_CS, HIGH);

    // This only works if I do this before we initialize the WDT, for some
    // reason. Not a huge priority to fix but I'd like to understand why
    // eventually.
    // 44100
    fk_assert(AudioInI2S.begin(8000, 32));

    leds.setup();
    watchdog.setup();
    bus.begin();
    power.setup();

    fk_assert(deviceId.initialize(bus));

    SerialNumber serialNumber;
    loginfof("Core", "Serial(%s)", serialNumber.toString());
    loginfof("Core", "DeviceId(%s)", deviceId.toString());
    loginfof("Core", "Hash(%s)", firmware_version_get());
    loginfof("Core", "Build(%s)", firmware_build_get());

    delay(10);

    #ifndef FK_DISABLE_FLASH
    fk_assert(flashStorage.initialize(Hardware::FLASH_PIN_CS));
    delay(100);
    #else
    loginfof("Core", "Serial flash is disabled.");
    #endif

    #ifdef FK_ENABLE_RADIO
    if (!radioService.setup(deviceId)) {
        loginfof("Core", "Radio service unavailable");
    }
    else {
        loginfof("Core", "Radio service ready");
    }
    #endif

    fk_assert(fileSystem.setup());

    watchdog.started();

    bus.begin();

    state.setDeviceId(deviceId.toString());

    clock.begin();

    FormattedTime nowFormatted{ clock.now() };
    loginfof("Core", "Now: %s", nowFormatted.toString());

    state.started();

    state.attachedModules()[0] = ModuleInfo{
        fk_module_ModuleType_SENSOR,
        8,
        18,
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
            {"audio_rms_avg", ""},
            {"audio_rms_min", ""},
            {"audio_rms_max", ""},
            {"audio_dbfs_avg", ""},
            {"audio_dbfs_min", ""},
            {"audio_dbfs_max", ""},
        },
        {
            {}, {}, {}, {},
            {}, {}, {}, {},
            {}, {}, {}, {},
            {}, {}, {}, {}, {}, {}
        }
    };

    state.doneScanning();

    readings.setup();
}

void NaturalistCoreModule::run() {
    SimpleNTP ntp(clock, wifi);
    Status status{ state, bus, leds };

    wifi.begin();

    background.append(ntp);

    auto tasks = to_parallel_task_collection(
        &status,
        &leds,
        &power,
        &watchdog,
        &liveData,
        &scheduler,
        &wifi,
        #ifdef FK_ENABLE_RADIO
        &radioService,
        #endif
        &discovery,
        &background,
        &servicing
    );

    while (true) {
        tasks.task();
    }
}

}

