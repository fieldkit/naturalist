#include <Arduino.h>

#include "config.h"
#include "restart_wizard.h"
#include "naturalist_core_module.h"

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

    randomSeed(RANDOM_SEED);
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
