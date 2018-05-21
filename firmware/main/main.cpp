#include <Arduino.h>

#include "config.h"
#include "restart_wizard.h"
#include "naturalist_core_module.h"
#include "hardware.h"
#include "seed.h"

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

    loginfof("Core", "Starting");

    #ifdef RANDOM_SEED
    randomSeed(RANDOM_SEED);
    loginfof("Core", "Seeded: %d", RANDOM_SEED);
    #endif
    firmware_version_set(FIRMWARE_GIT_HASH);
    firmware_build_set(FIRMWARE_BUILD);

    pinMode(fk::Hardware::PIN_PERIPH_ENABLE, OUTPUT);
    digitalWrite(fk::Hardware::PIN_PERIPH_ENABLE, LOW);
    delay(100);
    digitalWrite(fk::Hardware::PIN_PERIPH_ENABLE, HIGH);

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
    auto startupConfig = fk::StartupConfigurer{ coreModule.getState() };
    startupConfig.overrideEmptyNetworkConfigurations(fk::NetworkSettings{ false, networks });
    coreModule.run();
}

void loop() {
}

}
