#include <Arduino.h>

#include <fk-core.h>

#include "config.h"
#include "restart_wizard.h"

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
        debug_uart_set(Serial5);
    }

    firmware_version_set(FIRMWARE_GIT_HASH);

    debugfpln("Core", "Starting");
    debugfpln("Core", "Configured with UART fallback.");

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

    fk::CoreModule coreModule;
    coreModule.begin();
    coreModule.getState().configure(fk::NetworkSettings{ false, networks });
    coreModule.run();
}

void loop() {
}

}
