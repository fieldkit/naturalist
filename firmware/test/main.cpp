#include <fk-core.h>

#include <Wire.h>
#include <SPI.h>
#include <wiring_private.h>
#include <cstdarg>
#include <FuelGauge.h>
#include <WiFi101.h>
#include <../src/I2S.h>

#include <phylum/phylum.h>
#include <backends/arduino_sd/arduino_sd.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_TSL2561_U.h>
#include <Adafruit_MPL3115A2.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_SHT31.h>
#include <SerialFlash.h>
#include <RTClib.h>

constexpr const char LogName[] = "Check";

using Log = SimpleLog<LogName>;

Uart& gpsSerial = fk::Serial2;

class ModuleHardware {
public:
    static constexpr uint8_t PIN_RADIO_CS = 5;
    static constexpr uint8_t PIN_SD_CS = 12;
    static constexpr uint8_t PIN_WINC_CS = 7;
    static constexpr uint8_t PIN_WINC_IRQ = 16;
    static constexpr uint8_t PIN_WINC_RST = 15;
    static constexpr uint8_t PIN_WINC_EN = 38;
    static constexpr uint8_t PIN_WINC_WAKE = 8;

    static constexpr uint8_t PIN_FLASH_CS = (26u); // PIN_LED_TXL;
    static constexpr uint8_t PIN_PERIPH_ENABLE = (25u); // PIN_LED_RXL;
    static constexpr uint8_t PIN_MODULES_ENABLE = (A5);
    static constexpr uint8_t PIN_GPS_ENABLE = A4;

public:
    fk::TwoWireBus bno055Wire{ fk::Wire4and3 };
    Adafruit_SHT31 sht31Sensor;
    Adafruit_MPL3115A2 mpl3115a2Sensor;
    Adafruit_TSL2591 tsl2591Sensor{ 2591 };
    Adafruit_BNO055 bnoSensor{ 55, BNO055_ADDRESS_A, bno055Wire.twoWire() };
    SerialFlashChip serialFlash;

public:
    void setup() {
        pinMode(PIN_FLASH_CS, INPUT_PULLUP);
        pinMode(PIN_RADIO_CS, INPUT_PULLUP);
        pinMode(PIN_SD_CS, INPUT_PULLUP);
        pinMode(PIN_WINC_CS, INPUT_PULLUP);

        pinMode(PIN_WINC_EN, OUTPUT);
        digitalWrite(PIN_WINC_EN, LOW);

        pinMode(PIN_WINC_RST, OUTPUT);
        digitalWrite(PIN_WINC_RST, LOW);

        pinMode(PIN_FLASH_CS, OUTPUT);
        pinMode(PIN_RADIO_CS, OUTPUT);
        pinMode(PIN_SD_CS, OUTPUT);
        pinMode(PIN_WINC_CS, OUTPUT);

        digitalWrite(PIN_FLASH_CS, HIGH);
        digitalWrite(PIN_RADIO_CS, HIGH);
        digitalWrite(PIN_SD_CS, HIGH);
        digitalWrite(PIN_WINC_CS, HIGH);

        SPI.begin();
        Wire.begin();
        bno055Wire.begin();
    }

};

class Sensors {
private:
    ModuleHardware *hw;

public:
    Sensors(ModuleHardware &hw) : hw(&hw) {
    }

public:
    void takeReading() {
        auto shtTemperature = hw->sht31Sensor.readTemperature();
        auto shtHumidity = hw->sht31Sensor.readHumidity();

        Log::info("sensors: %fC %f%%", shtTemperature, shtHumidity);

        auto pressurePascals = hw->mpl3115a2Sensor.getPressure();
        auto altitudeMeters = hw->mpl3115a2Sensor.getAltitude();
        auto mplTempCelsius = hw->mpl3115a2Sensor.getTemperature();
        auto pressureInchesMercury = pressurePascals / 3377.0;

        Log::info("sensors: %fC %fpa %f\"/Hg %fm", mplTempCelsius, pressurePascals, pressureInchesMercury, altitudeMeters);

        auto fullLuminosity = hw->tsl2591Sensor.getFullLuminosity();
        auto ir = fullLuminosity >> 16;
        auto full = fullLuminosity & 0xFFFF;
        auto lux = hw->tsl2591Sensor.calculateLux(full, ir);

        Log::info("sensors: ir(%lu) full(%lu) visible(%lu) lux(%f)", ir, full, full - ir, lux);

        #if defined(FK_ENABLE_BNO05)
        uint8_t system = 0, gyro = 0, accel = 0, mag = 0;
        hw->bnoSensor.getCalibration(&system, &gyro, &accel, &mag);

        sensors_event_t event;
        hw->bnoSensor.getEvent(&event);

        Log::info("sensors: cal(%d, %d, %d, %d) xyz(%f, %f, %f)", system, gyro, accel, mag, event.orientation.x, event.orientation.y, event.orientation.z);
        #endif
    }
};

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

class Check {
private:
    ModuleHardware *hw;

public:
    Check(ModuleHardware &hw) : hw(&hw) {
    }

public:
    bool sht31() {
        if (!hw->sht31Sensor.begin()) {
            Log::info("SHT31 FAILED");
            return false;
        }

        auto shtTemperature = hw->sht31Sensor.readTemperature();
        Log::info("SHT31 %f", shtTemperature);

        Log::info("SHT31 PASSED");
        return true;
    }

    bool mpl3115a2() {
        if (!hw->mpl3115a2Sensor.begin()) {
            Log::info("MPL3115A2 FAILED");
            return false;
        }

        auto pressurePascals = hw->mpl3115a2Sensor.getPressure();
        Log::info("MPL3115A2 %f", pressurePascals);

        Log::info("MPL3115A2 PASSED");
        return true;
    }

    bool tsl2591() {
        if (!hw->tsl2591Sensor.begin()) {
            Log::info("TSL25911FN FAILED");
            return false;
        }

        Log::info("TSL25911FN PASSED");
        return true;
    }

    bool bno055() {
        Log::info("BNO055 Checking...");

        if (!hw->bnoSensor.begin()) {
            Log::info("BNO055 FAILED");
            return false;
        }

        hw->bnoSensor.setExtCrystalUse(true);

        Log::info("BNO055 PASSED");
        return true;
    }

    bool sph0645() {
        Log::info("SPH0645 Checking...");

        if (!I2S.begin(I2S_PHILIPS_MODE, 16000, 32)) {
            Log::info("SPH0645 FAILED");
            return false;
        }

        Log::info("SPH0645 PASSED");
        return true;
    }

    bool flashMemory() {
        Log::info("Checking flash memory (%d)...", ModuleHardware::PIN_FLASH_CS);

        if (!hw->serialFlash.begin(ModuleHardware::PIN_FLASH_CS)) {
            Log::info("Flash memory FAILED");
            return false;
        }

        uint8_t buffer[256] = { 0 };
        hw->serialFlash.readID(buffer);
        if (buffer[0] == 0) {
            Log::info("Flash memory FAILED");
            return false;
        }

        uint32_t chipSize = hw->serialFlash.capacity(buffer);
        if (chipSize == 0) {
            Log::info("Flash memory FAILED");
            return false;
        }

        if (false) {
            if (chipSize > 0) {
                Log::info("Erasing ALL Flash Memory (%lu)", chipSize);

                hw->serialFlash.eraseAll();

                delay(1000);

                uint32_t dotMillis = millis();
                while (hw->serialFlash.ready() == false) {
                    if (millis() - dotMillis > 1000) {
                        dotMillis = dotMillis + 1000;
                    }
                }
                Log::info("Erase completed");
            }
        }

        Log::info("Flash memory PASSED");
        return true;
    }

    bool sdCard() {
        Log::info("Checking SD...");
        phylum::Geometry g;
        phylum::ArduinoSdBackend storage;
        if (!storage.initialize(g, ModuleHardware::PIN_SD_CS)) {
            Log::info("SD FAILED (to initialize)");
            return false;
        }

        if (!storage.open()) {
            Log::info("SD FAILED (to open)");
            return false;
        }

        Log::info("SD PASSED");

        return true;
    }

    bool gps() {
        Log::info("Checking gps...");

        fk::SerialPort gpsPort{ gpsSerial };
        gpsPort.begin(9600);

        uint32_t charactersRead = 0;
        uint32_t start = millis();
        while (millis() - start < 5 * 1000 && charactersRead < 100)  {
            while (gpsSerial.available()) {
                Serial.print((char)gpsSerial.read());
                charactersRead++;
            }
        }

        if (charactersRead < 100) {
            Log::info("GPS FAILED");
            return false;
        }

        Serial.println();

        Log::info("GPS PASSED");
        return true;
    }


    bool wifi() {
        Log::info("Checking wifi...");

        delay(500);

        digitalWrite(ModuleHardware::PIN_WINC_RST, HIGH);

        WiFi.setPins(ModuleHardware::PIN_WINC_CS, ModuleHardware::PIN_WINC_IRQ, ModuleHardware::PIN_WINC_RST);

        digitalWrite(ModuleHardware::PIN_WINC_EN, LOW);
        delay(50);

        digitalWrite(ModuleHardware::PIN_WINC_EN, HIGH);

        delay(50);

        if (WiFi.status() == WL_NO_SHIELD) {
            Log::info("Wifi FAILED");
            return false;
        }

        Log::info("Wifi firmware version: ");
        auto fv = WiFi.firmwareVersion();
        Log::info("Wifi version: %s", fv);
        Log::info("Wifi PASSED");

        return true;
    }

    bool check() {
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

        Log::info("Enabling peripherals!");
        pinMode(ModuleHardware::PIN_PERIPH_ENABLE, OUTPUT);
        pinMode(ModuleHardware::PIN_MODULES_ENABLE, OUTPUT);
        pinMode(ModuleHardware::PIN_GPS_ENABLE, OUTPUT);
        digitalWrite(ModuleHardware::PIN_PERIPH_ENABLE, LOW);
        digitalWrite(ModuleHardware::PIN_MODULES_ENABLE, LOW);
        digitalWrite(ModuleHardware::PIN_GPS_ENABLE, LOW);
        delay(500);
        digitalWrite(ModuleHardware::PIN_PERIPH_ENABLE, HIGH);
        digitalWrite(ModuleHardware::PIN_MODULES_ENABLE, HIGH);
        digitalWrite(ModuleHardware::PIN_GPS_ENABLE, HIGH);

        auto failures = false;
        if (!macEeprom()) {
            failures = true;
        }
        if (!rtc()) {
            failures = true;
        }
        if (!fuelGauge()) {
            failures = true;
        }
        if (!mpl3115a2()) {
            failures = true;
        }
        if (!tsl2591()) {
            failures = true;
        }
        if (!sht31()) {
            failures = true;
        }
        if (!sph0645()) {
            failures = true;
        }
        #if defined(FK_ENABLE_BNO05)
        if (!bno055()) {
            failures = true;
        }
        #endif
        if (!gps()) {
            failures = true;
        }
        if (!sdCard()) {
            failures = true;
        }
        if (!wifi()) {
            failures = true;
        }
        if (!flashMemory()) {
            failures = true;
        }

        return !failures;
    }

    bool fuelGauge() {
        FuelGauge gauge;

        gauge.powerOn();

        if (gauge.version() != 3) {
            Log::info("Gauge FAILED");
            return true;
        }

        Log::info("Gauge PASSED");
        return true;
    }

    bool rtc() {
        RTC_PCF8523 rtc;
        if (!rtc.begin()) {
            Log::info("RTC FAILED");
            return false;
        }
        Log::info("RTC PASSED");
        return true;
    }

    bool macEeprom() {
        MacEeprom macEeprom;
        uint8_t id[8] = { 0 };

        Log::info("READING");

        auto success = macEeprom.read128bMac(id);
        if (!success) {
            Log::info("128bMAC FAILED");
            return false;
        }

        Log::info("128bMAC: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7]);

        Log::info("128bMAC PASSED");

        return success;
    }

};

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(100);
    }

    Log::info("Setup");

    ModuleHardware hw;
    hw.setup();

    Log::info("Begin");

    fk::Leds leds;
    leds.setup();

    Check check(hw);
    auto success = check.check();
    if (success) {
        leds.notifyHappy();
    }
    else {
        leds.notifyFatal();
    }

    Log::info("Done");

    Sensors sensors(hw);
    while (true) {
        leds.task();
        if (success) {
            sensors.takeReading();
        }
    }

    delay(100);
}

void loop() {

}
