#include <alogging/alogging.h>

#include "check_naturalist.h"
#include "board_definition.h"

using namespace fk;

constexpr const char LogName[] = "Check";

using Log = SimpleLog<LogName>;

void setup() {
    Serial5.begin(115200);
    Serial.begin(115200);

    while (!Serial && millis() < 5 * 1000) {
        delay(10);
    }

    if (!Serial) {
        // The call to end here seems to free up some memory.
        Serial.end();
        log_uart_set(Serial5);
    }

    CheckNaturalist check;
    check.setup();

    if (!check.check()) {
        check.leds().notifyFatal();

        while (true) {
            check.task();
            delay(10);
        }
    }

    check.leds().notifyHappy();

    while (true) {
        check.task();
        delay(10);
    }
}

void loop() {

}
