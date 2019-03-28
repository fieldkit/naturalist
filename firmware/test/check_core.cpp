#include "check_core.h"
#include "board_definition.h"
#include "config.h"

#include <RH_RF95.h>

namespace fk {

constexpr const char LogName[] = "Check";

using Log = SimpleLog<LogName>;

class MacEeprom {
private:
    uint8_t address;

public:
    MacEeprom() : address(0x50) {
    }

public:
    bool read128bMac(uint8_t *id) {
        Wire.beginTransmission(address);
        Wire.write(0xf8);
        Wire.endTransmission();
        Wire.requestFrom(address, 8);

        uint8_t index = 0;
        while (Wire.available()){
            id[index++] = Wire.read();
        }

        return true;
    }
};

void CheckCore::setup() {
    #ifdef PIN_LED_RXL
    Log::info("Please undefine PIN_LED_RXL in variant.h.");
    #else
    Log::info("PIN_LED_RXL is undefined. Good!");
    #endif

    #ifdef PIN_LED_TXL
    Log::info("Please undefine PIN_LED_TXL in variant.h, otherwise SerialFlash and other SPI devices may work incorrectly.");
    #else
    Log::info("PIN_LED_TXL is undefined. Good!");
    #endif

    leds_.setup();

    board.disable_everything();

    delay(100);

    board.enable_everything();

    delay(100);
}

bool CheckCore::fuelGauge() {
    Log::info("Checking gauge...");

    BatteryGauge gauge;

    if (!gauge.available()) {
        Log::info("Gauge FAILED (MISSING)");
        return false;
    }

    for (auto i = 0; i < 10; ++i) {
        auto reading = gauge_.read();

        Log::info("Battery: v=%fmv i=%fmA cc=%fmAh (%fmAh) c=%d",
                  reading.voltage, reading.ma, reading.coulombs,
                  reading.coulombs - previous_, reading.counter);

        if (reading.voltage > 2500.0f) {
            Log::info("Gauge PASSED");
            return true;
        }

        delay(500);
    }

    Log::info("Gauge FAILED (VOLTAGE)");
    return false;
}

bool CheckCore::flashMemory() {
    Log::info("Checking flash memory...");

    if (!SerialFlash.begin(Hardware::FLASH_PIN_CS)) {
        Log::info("Flash memory FAILED");
        return false;
    }

    uint8_t buffer[256];

    SerialFlash.readID(buffer);
    if (buffer[0] == 0) {
        Log::info("Flash memory FAILED");
        return false;
    }

    auto chipSize = SerialFlash.capacity(buffer);
    if (chipSize == 0) {
        Log::info("Flash memory FAILED");
        return false;
    }

    auto blockSize = SerialFlash.blockSize();

    Log::info("Read Chip Identification:");
    Log::info("  JEDEC ID:     %x %x %x", buffer[0], buffer[1], buffer[2]);
    Log::info("  Part Nummber: %s", id2chip(buffer));
    Log::info("  Memory Size:  %lu bytes Block Size: %lu bytes", chipSize, blockSize);
    Log::info("Flash memory PASSED");

    if (chipSize > 0) {
        if (false) {
            Log::info("Erasing ALL Flash Memory (%lu)", chipSize);

            SerialFlash.eraseAll();

            auto started = millis();
            while (SerialFlash.ready() == false) {
                if (millis() - started > 1000) {
                    started = millis();
                }
            }
        }
        else {
            for (auto i = 0; i < 10; ++i) {
                Log::info("Erasing block %d (%lu)", i, i * blockSize);
                SerialFlash.eraseBlock(i * blockSize);
            }
        }

        Log::info("Erase completed");
    }

    return true;
}

bool CheckCore::gps() {
    Log::info("Checking gps...");

    Uart& gpsSerial = Serial2;
    SerialPort gpsPort{ gpsSerial };
    gpsPort.begin(9600);

    uint32_t charactersRead = 0;
    uint32_t start = millis();
    while (millis() - start < 5 * 1000 && charactersRead < 100)  {
        while (gpsSerial.available()) {
            Serial.print((char)gpsSerial.read());
            charactersRead++;
        }
    }

    if (Serial) {
        Serial.println();
    }
    else {
        Serial5.println();
    }

    if (charactersRead < 100) {
        Log::info("GPS FAILED");
        return false;
    }

    Log::info("GPS PASSED");

    return true;
}

bool CheckCore::sdCard() {
    Log::info("Checking SD...");

    phylum::Geometry g;
    phylum::ArduinoSdBackend storage;
    if (!storage.initialize(g, Hardware::SD_PIN_CS)) {
        Log::info("SD FAILED (to open)");
        return false;
    }

    if (!storage.open()) {
        Log::info("SD FAILED");
        return false;
    }

    digitalWrite(Hardware::SD_PIN_CS, HIGH);
    Log::info("SD PASSED");

    return true;
}

bool CheckCore::radio() {
    Log::info("Checking radio...");

    RH_RF95 rf95(Hardware::RFM95_PIN_CS, Hardware::RFM95_PIN_D0);

    if (!rf95.init()) {
        Log::info("Radio FAILED");
        return false;
    }

    digitalWrite(Hardware::RFM95_PIN_CS, HIGH);
    Log::info("Radio PASSED");

    return true;
}

bool CheckCore::wifi() {
    Log::info("Checking wifi...");

    // TODO: Move this into CoreBoard?
    WiFi.setPins(Hardware::WIFI_PIN_CS, Hardware::WIFI_PIN_IRQ, Hardware::WIFI_PIN_RST);

    if (WiFi.status() == WL_NO_SHIELD) {
        Log::info("Wifi FAILED");
        return false;
    }

    Log::info("Wifi firmware version: ");
    auto fv = WiFi.firmwareVersion();
    Log::info("Version: %s", fv);
    Log::info("Wifi PASSED");

    #if defined(FK_CONFIG_WIFI_1_SSID) && defined(FK_CONFIG_WIFI_1_PASSWORD)
    Log::info("Connecting to %s", FK_CONFIG_WIFI_1_SSID);
    if (WiFi.begin(FK_CONFIG_WIFI_1_SSID, FK_CONFIG_WIFI_1_PASSWORD) != WL_CONNECTED) {
        Log::info("Connection failed!");
        return false;
    }
    Log::info("Connected. Syncing time, check for a battery!");

    clock.begin();

    SimpleNTP ntp(clock);

    ntp.enqueued();

    while (true) {
        if (!simple_task_run(ntp)) {
            break;
        }
    }

    #else
    Log::info("Skipping connection test, no config.");
    #endif

    return true;
}

bool CheckCore::rtc() {
    RTC_PCF8523 rtc;

    if (!rtc.begin()) {
        Log::info("RTC FAILED");
        return false;
    }
    Log::info("RTC PASSED");

    return true;
}

bool CheckCore::macEeprom() {
    MacEeprom macEeprom;
    uint8_t id[8] = { 0 };

    auto success = macEeprom.read128bMac(id);
    if (!success) {
        Log::info("128bMAC FAILED");
        return false;
    }

    Log::info("128bMAC: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);

    Log::info("128bMAC PASSED");

    return success;
}

bool CheckCore::check() {
    auto have_gauge = false;
    auto have_sd = false;

    success_ = true;
    caution_ = false;
    sampling_ = true;

    #if defined(FK_ENABLE_FUEL_GAUGE)
    if (!fuelGauge()) {
        have_gauge = false;
        success_ = false;
        sampling_ = false;
    }
    #else
    Log::info("Fuel gauge disabled.");
    #endif
    success_ = macEeprom() && success_;
    success_ = rtc() && success_;
    success_ = flashMemory() && success_;

    if (success_) {
        Log::info("Top PASSED");
        leds().notifyCaution();
        leds().task();
    }
    else {
        leds().notifyFatal();
        leds().task();
    }

    #if defined(FK_ENABLE_RADIO)
    success_ = radio() && success_;
    #else
    Log::info("Radio disabled");
    #endif
    success_ = gps() && success_;
    if (!sdCard()) {
        have_sd = false;
        success_ = false;
    }
    success_ = wifi() && success_;

    leds().off();

    if (success_) {
        Log::info("test: SUCCESS");
        leds().notifyHappy();
    }
    else {
        if (!have_gauge) {
            Log::info("test: FATAL");
            leds().notifyFatal();
        }
        else if (!have_sd) {
            Log::info("test: PARTIAL SUCCESS (SD)");
            leds().notifyCaution();
        }
        else {
            Log::info("test: FATAL");
            leds().notifyFatal();
        }
    }

    return success_;
}

void CheckCore::task() {
    leds_.task();

    if (sampling()) {
        if (toggle_peripherals()) {
            if (fk_uptime() - toggled_ > 20000) {
                enabled_ = !enabled_;

                if (enabled_) {
                    Log::info("Enable Peripherals");
                    board.enable_spi();
                    board.enable_gps();
                    board.enable_wifi();
                    leds().notifyHappy();
                }
                else {
                    Log::info("Disable Peripherals");
                    leds().off();
                    board.disable_wifi();
                    board.disable_gps();
                    board.disable_spi();
                }

                toggled_ = fk_uptime();
            }
        }

        if (fk_uptime() - checked_ > 2500) {
            sample();
            checked_ = fk_uptime();
        }
    }
}

void CheckCore::sample() {
    auto reading = gauge_.read();
    Log::info("Battery: v=%fmv i=%fmA cc=%fmAh (%fmAh) c=%d",
              reading.voltage, reading.ma, reading.coulombs,
              reading.coulombs - previous_, reading.counter);
    previous_ = reading.coulombs;
}

const char *CheckCore::id2chip(const unsigned char *id) {
    if (id[0] == 0xEF) {
        // Winbond
        if (id[1] == 0x40) {
            if (id[2] == 0x14) return "W25Q80BV";
            if (id[2] == 0x15) return "W25Q16DV";
            if (id[2] == 0x17) return "W25Q64FV";
            if (id[2] == 0x18) return "W25Q128FV";
            if (id[2] == 0x19) return "W25Q256FV";
        }
    }
    if (id[0] == 0x01) {
        // Spansion
        if (id[1] == 0x02) {
            if (id[2] == 0x16) return "S25FL064A";
            if (id[2] == 0x19) return "S25FL256S";
            if (id[2] == 0x20) return "S25FL512S";
        }
        if (id[1] == 0x20) {
            if (id[2] == 0x18) return "S25FL127S";
        }
        if (id[1] == 0x40) {
            if (id[2] == 0x15) return "S25FL116K";
        }
    }
    if (id[0] == 0xC2) {
        // Macronix
        if (id[1] == 0x20) {
            if (id[2] == 0x18) return "MX25L12805D";
        }
    }
    if (id[0] == 0x20) {
        // Micron
        if (id[1] == 0xBA) {
            if (id[2] == 0x20) return "N25Q512A";
            if (id[2] == 0x21) return "N25Q00AA";
        }
        if (id[1] == 0xBB) {
            if (id[2] == 0x22) return "MT25QL02GC";
        }
    }
    if (id[0] == 0xBF) {
        // SST
        if (id[1] == 0x25) {
            if (id[2] == 0x02) return "SST25WF010";
            if (id[2] == 0x03) return "SST25WF020";
            if (id[2] == 0x04) return "SST25WF040";
            if (id[2] == 0x41) return "SST25VF016B";
            if (id[2] == 0x4A) return "SST25VF032";
        }
        if (id[1] == 0x25) {
            if (id[2] == 0x01) return "SST26VF016";
            if (id[2] == 0x02) return "SST26VF032";
            if (id[2] == 0x43) return "SST26VF064";
        }
    }
    return "(unknown chip)";
}

}
