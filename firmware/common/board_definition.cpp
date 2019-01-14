#include "board_definition.h"

#include "hardware.h"

namespace fk {

CoreBoard board{
    {
        {
            Hardware::PERIPHERALS_ENABLE_PIN,
            Hardware::FLASH_PIN_CS,
            {
                Hardware::FLASH_PIN_CS,
                Hardware::WIFI_PIN_CS,
                Hardware::RFM95_PIN_CS,
                Hardware::SD_PIN_CS,
            },
            {
                Hardware::PERIPHERALS_ENABLE_PIN,
                Hardware::GPS_ENABLE_PIN,
                0,
                0,
            }
        },
        Hardware::SD_PIN_CS,
        Hardware::WIFI_PIN_CS,
        Hardware::WIFI_PIN_EN,
        Hardware::GPS_ENABLE_PIN,
        Hardware::MODULES_ENABLE_PIN,
    }
};

}
