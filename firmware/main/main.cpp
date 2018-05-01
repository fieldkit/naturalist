#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_SHT31.h>

#include <fk-core.h>

#include "config.h"
#include "restart_wizard.h"
#include "hardware.h"
#include "two_wire.h"
#include "seed.h"

namespace fk {

class NaturalistReadings : public Task {
private:
    CoreState *state;
    TwoWireBus bno055Wire{ Wire4and3 };
    Adafruit_SHT31 sht31Sensor;
    Adafruit_MPL3115A2 mpl3115a2Sensor;
    Adafruit_TSL2591 tsl2591Sensor{ 2591 };
    Adafruit_BNO055 bnoSensor{ 55, BNO055_ADDRESS_A, &Wire4and3 };
    bool hasBno055{ false };

public:
    NaturalistReadings(CoreState &state) : Task("Naturalist"), state(&state) {
    }

public:
    void setup();
    void enqueued() override;
    TaskEval task() override;

};

void NaturalistReadings::setup() {
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

    bno055Wire.begin();

    if (!bnoSensor.begin()) {
        log("BNO055 FAILED");
    } else {
        hasBno055 = true;
        bnoSensor.setExtCrystalUse(true);
    }
}

void NaturalistReadings::enqueued() {
}

TaskEval NaturalistReadings::task() {
    log("Ready!");

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
    };
    auto time = clock.getTime();
    for (auto i = 0; i < sizeof(values) / sizeof(float); ++i) {
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

    return TaskEval::done();
}

class NaturalistCoreModule {
private:
    StaticPool<384> appPool{"AppPool"};
    StaticPool<384> modulesPool{"ModulesPool"};
    StaticPool<128> dataPool{"DataPool"};

    Supervisor<5> background{ true };
    Supervisor<5> servicing{ true };

    TwoWireBus bus{ Wire };
    FileSystem fileSystem{ bus, dataPool };
    Watchdog watchdog{ leds };
    Power power{ state };
    SerialFlashChip serialFlash;
    FlashStorage storage{ serialFlash };
    CoreState state{ storage, fileSystem.getData() };
    Leds leds;
    NaturalistReadings readings{ state };
    SerialPort gpsPort{ Serial2 };
    ReadGps readGps{ state, gpsPort };
    PeriodicTask periodics[2] {
        fk::PeriodicTask{ 20 * 1000, readGps },
        fk::PeriodicTask{ 30 * 1000, readings },
    };
    Scheduler scheduler{state, clock, background, periodics};
    LiveData liveData{readings, state};
    WifiConnection connection;
    ModuleCommunications moduleCommunications{ bus, background, modulesPool };
    AppServicer appServicer{bus, liveData, state, scheduler, fileSystem.getReplies(), connection, moduleCommunications, appPool};
    Wifi wifi{state, connection, appServicer, servicing};
    Discovery discovery{ bus, wifi };

public:
    void begin();
    void run();

public:
    CoreState &getState() {
        return state;
    }

};

void NaturalistCoreModule::begin() {
    pinMode(Hardware::SD_PIN_CS, OUTPUT);
    pinMode(Hardware::WIFI_PIN_CS, OUTPUT);
    pinMode(Hardware::RFM95_PIN_CS, OUTPUT);
    pinMode(Hardware::FLASH_PIN_CS, OUTPUT);

    digitalWrite(Hardware::SD_PIN_CS, HIGH);
    digitalWrite(Hardware::WIFI_PIN_CS, HIGH);
    digitalWrite(Hardware::RFM95_PIN_CS, HIGH);
    digitalWrite(Hardware::FLASH_PIN_CS, HIGH);

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
    fk_assert(serialFlash.begin(Hardware::FLASH_PIN_CS));
    fk_assert(storage.setup());

    delay(100);
    #else
    loginfof("Core", "Serial flash is disabled.");
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
            {}, {}, {}, {},
            {}, {}, {}, {},
            {}, {}, {}, {},
        }
    };

    state.doneScanning();

    readings.setup();
}

void NaturalistCoreModule::run() {
    SimpleNTP ntp(clock, wifi);
    Status status{ state, bus };

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
        &discovery,
        &background,
        &servicing
    );

    while (true) {
        tasks.task();
    }
}

}

extern "C" {

void setup() {
    Serial.begin(115200);

    while (!Serial && millis() < 2000) {
        delay(100);
    }

    if (!Serial) {
        // The call to end here seems to free up some memory.
        Serial.end();
        Serial5.begin(115200);
        log_uart_set(Serial5);
    }

    firmware_version_set(FIRMWARE_GIT_HASH);
    firmware_build_set(FIRMWARE_BUILD);

    loginfof("Core", "Starting");
    loginfof("Core", "Configured with UART fallback.");

    fk::restartWizard.startup();

    fk::NetworkInfo networks[] = {
        {
            FK_CONFIG_WIFI_1_SSID,
            FK_CONFIG_WIFI_1_PASSWORD,
        },
        {
            FK_CONFIG_WIFI_2_SSID,
            FK_CONFIG_WIFI_2_PASSWORD,
        }
    };

    fk::NaturalistCoreModule coreModule;
    coreModule.begin();
    coreModule.getState().configure(fk::NetworkSettings{ false, networks });
    coreModule.run();
}

void loop() {
}

}
