#include <alogging/alogging.h>

#include "check_naturalist.h"
#include "board_definition.h"

using namespace fk;

constexpr const char LogName[] = "Check";

using Log = SimpleLog<LogName>;

void setup() {
    Serial.begin(115200);

    while (!Serial && millis() < 2 * 1000) {
        delay(100);
    }

    if (!Serial) {
        // The call to end here seems to free up some memory.
        Serial.end();
        Serial5.begin(115200);
        log_uart_set(Serial5);
    }

    CheckNaturalist check;
    check.setup();

    if (!check.check()) {
        check.leds().notifyFatal();

        board.disable_everything();

        while (true) {
            check.task();
            delay(10);
        }
    }

    check.leds().notifyHappy();

    BatteryGauge gauge;

    auto checked = fk_uptime();
    auto toggled = fk_uptime();
    auto enabled = true;
    auto previous = 0.0f;

    while (true) {
        check.task();
        delay(10);

        if (fk_uptime() - toggled > 20000) {
            enabled = !enabled;

            if (enabled) {
                Log::info("Enable Peripherals");
                board.enable_spi();
                board.enable_gps();
                board.enable_wifi();
                check.leds().notifyHappy();
            }
            else {
                Log::info("Disable Peripherals");
                check.leds().off();
                board.disable_wifi();
                board.disable_gps();
                board.disable_spi();
            }

            toggled = fk_uptime();
        }

        if (fk_uptime() - checked > 2500) {
            auto reading = gauge.read();

            Log::info("Battery: v=%fmv i=%fmA cc=%fmAh (%fmAh) c=%d",
                      reading.voltage, reading.ma, reading.coulombs,
                      reading.coulombs - previous, reading.counter);

            checked = fk_uptime();
            previous = reading.coulombs;
        }
    }
}

void loop() {

}
