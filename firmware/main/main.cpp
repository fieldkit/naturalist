/**
 * @file
 */
#include <Arduino.h>
#include <SEGGER_RTT.h>

#include <fk-core.h>

#include "platform.h"
#include "restart_wizard.h"
#include "initialized.h"
#include "readings.h"
#include "alogging/../printf.h"

#include "seed.h"
#include "config.h"

extern "C" {

namespace fk {

SensorInfo sensors[] = {
    { "temp_1", "°C" },
    { "humidity", "%" },
    { "temp_2", "°C" },
    { "pressure", "pa" },
    { "altitude", "m" },
    { "light_ir", "" },
    { "light_visible", "" },
    { "light_lux", "" },
    { "imu_cal", "" },
    { "imu_orien_x", "" },
    { "imu_orien_y", "" },
    { "imu_orien_z", "" },
    { "audio_rms_avg", "" },
    { "audio_rms_min", "" },
    { "audio_rms_max", "" },
    { "audio_dbfs_avg", "" },
    { "audio_dbfs_min", "" },
    { "audio_dbfs_max", "" },
};

SensorReading readings[18];

ModuleInfo module = {
    fk_module_ModuleType_SENSOR,
    8,
    18,
    1,
    "FkNat",
    "fk-naturalist",
    sensors,
    readings
};

class ConfigureDevice : public MainServicesState {
public:
    const char *name() const override {
        return "ConfigureDevice";
    }

public:
    void task() override {
        auto state = services().state;

        #if defined(FK_CONFIG_WIFI_1_SSID) && defined(FK_CONFIG_WIFI_2_SSID)
        NetworkInfo networks[2] = {
            {
                FK_CONFIG_WIFI_1_SSID,
                FK_CONFIG_WIFI_1_PASSWORD,
            },
            {
                FK_CONFIG_WIFI_2_SSID,
                FK_CONFIG_WIFI_2_PASSWORD,
            }
        };

        state->configure(NetworkSettings{ false, networks });
        log("Configured compile time networks.");
        #endif

        state->configure(module);
        state->doneScanning();

        CoreFsm::state<TakeNaturalistReadings>().setup();

        log("Configured");

        transit<Initialized>();
    }
};

}

static void setup_serial();
static void setup_env();

static size_t write_log(const LogMessage *m, const char *fstring, va_list args) {
    char message_buffer[256];

    SEGGER_RTT_LOCK();

    auto uart = log_uart_get();
    auto level = alog_get_log_level((LogLevels)m->level);
    auto f = "%08" PRIu32 " %-6s %s" ": ";
    alogging_snprintf(message_buffer, sizeof(message_buffer), f, m->uptime, level, m->facility);
    uart->print(message_buffer);

    SEGGER_RTT_WriteString(0, message_buffer);

    auto n = alogging_vsnprintf(message_buffer, sizeof(message_buffer), fstring, args);
    auto s = message_buffer + std::min((size_t)n, sizeof(message_buffer) - 1);
    for ( ; s > message_buffer; s--) {
        if (*s == '\n') {
            *s = 0;
            break;
        }
    }
    uart->println(message_buffer);

    SEGGER_RTT_WriteString(0, message_buffer);
    SEGGER_RTT_WriteString(0, "\n");
    SEGGER_RTT_UNLOCK();

    return true;
}

void setup() {
    #ifdef FK_DEBUG_MTB_ENABLE
    REG_MTB_POSITION = ((uint32_t)(mtb - REG_MTB_BASE)) & 0xFFFFFFF8;
    REG_MTB_FLOW = ((uint32_t)mtb + DEBUG_MTB_SIZE * sizeof(uint32_t)) & 0xFFFFFFF8;
    REG_MTB_MASTER = 0x80000000 + 6;
    #endif

    SEGGER_RTT_Init();
    SEGGER_RTT_SetFlagsUpBuffer(0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    setup_serial();
    setup_env();

    log_configure_writer(write_log);

    if (false) {
        fk::restartWizard.startup();
    }
}

void loop() {
    fk::ConfigurableStates states{
        fk::CoreFsm::deferred<fk::ConfigureDevice>(),
        fk::CoreFsm::deferred<fk::TakeNaturalistReadings>()
    };
    fk::CoreModule coreModule(states);
    coreModule.run();
}

static void setup_serial() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);

        #ifndef FK_DEBUG_UART_REQUIRE_CONSOLE
        if (fk::fk_uptime() > 2000) {
            break;
        }
        #endif
    }

    #ifdef FK_DEBUG_UART_FALLBACK
    if (!Serial) {
        // The call to end here seems to free up some memory.
        // Serial.end();
        Serial5.begin(115200);
        log_uart_set(Serial5);
    }
    #endif
}

static void setup_env() {
    randomSeed(RANDOM_SEED);
    firmware_version_set(FIRMWARE_GIT_HASH);
    firmware_build_set(FIRMWARE_BUILD);
    firmware_compiled_set(DateTime(__DATE__, __TIME__).unixtime());
}

#ifdef FK_DEBUG_MTB_ENABLE

#define DEBUG_MTB_SIZE 256
__attribute__((__aligned__(DEBUG_MTB_SIZE * sizeof(uint32_t)))) uint32_t mtb[DEBUG_MTB_SIZE];

void HardFault_Handler(void) {
    // Turn off the micro trace buffer so we don't fill it up in the infinite loop below.
    REG_MTB_MASTER = 0x00000000 + 6;
    while (true) {
    }
}

#endif

}
