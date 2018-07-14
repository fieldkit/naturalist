#include "naturalist_core_module.h"

namespace fk {

constexpr const char LogName[] = "Core";

using Logger = SimpleLog<LogName>;

void NaturalistCoreModule::begin() {
    MainServicesState::services(mainServices);
    WifiServicesState::services(wifiServices);

    fsm_list::start();

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
    // NOTE: FkNaturalist Specific
    fk_assert(AudioInI2S.begin(8000, 32));

    leds.setup();
    watchdog.setup();
    bus.begin();
    power.setup();
    clock.begin();
    button.enqueued();

    fk_assert(deviceId.initialize(bus));
    state.setDeviceId(deviceId.toString());

    #ifdef FK_CORE_GENERATION_2
    Logger::info("Cycling peripherals.");
    pinMode(Hardware::PIN_PERIPH_ENABLE, OUTPUT);
    digitalWrite(Hardware::PIN_PERIPH_ENABLE, LOW);
    delay(500);
    digitalWrite(Hardware::PIN_PERIPH_ENABLE, HIGH);
    delay(500);
    #else
    Logger::info("Peripherals always on.");
    #endif

    #ifdef FK_ENABLE_FLASH
    fk_assert(flashStorage.initialize(Hardware::FLASH_PIN_CS));
    #else
    Logger::info("Flash memory disabled");
    #endif

    #ifdef FK_ENABLE_RADIO
    if (!radioService.setup(deviceId)) {
        Logger::info("Radio service unavailable");
    }
    else {
        Logger::info("Radio service ready");
    }
    #else
    Logger::info("Radio service disabled");
    #endif

    fk_assert(fileSystem.setup());

    SerialNumber serialNumber;
    Logger::info("Serial(%s)", serialNumber.toString());
    Logger::info("DeviceId(%s)", deviceId.toString());
    Logger::info("Hash(%s)", firmware_version_get());
    Logger::info("Build(%s)", firmware_build_get());
    Logger::info("API(%s)", WifiApiUrlIngestionStream);

    FormattedTime nowFormatted{ clock.now() };
    Logger::info("Now: %s", nowFormatted.toString());

    watchdog.started();
    state.started();
    scheduler.started();

    // NOTE: FkNaturalist Specific:

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
    while (true) {
        CoreDevice::current().task();
    }
}

}

